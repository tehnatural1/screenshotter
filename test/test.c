#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <mach-o/dyld.h>
#include <string.h>
#include <limits.h>
#include <ptrauth.h>
// #include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/dyld_images.h>
#include <mach/vm_map.h>
#include <mach/mach.h>
#include <sys/mman.h>
#include <dlfcn.h>

#include <AudioToolbox/AudioUnit.h>
#include <AudioToolbox/AUComponent.h>

#include <CoreAudio/CoreAudio.h>

// CoreAudioProcess()

typedef OSStatus ( *audio_unit_process_t )( AudioUnit inUnit, 
			                                AudioUnitRenderActionFlags* __nullable ioActionFlags, 
									        const AudioTimeStamp* inTimeStamp, 
									        UInt32 inNumberFrames, 
									        AudioBufferList* ioData );

audio_unit_process_t AudioUnitProcess_orig = NULL;
// audio_unit_process_t AudioUnitProcess_hook = NULL;

audio_unit_process_t
    AudioUnitProcess_hook
    (
        AudioUnit inUnit, 
        AudioUnitRenderActionFlags* __nullable ioActionFlags, 
        const AudioTimeStamp* inTimeStamp, 
        UInt32 inNumberFrames, 
        AudioBufferList* ioData
    )
{
    OSStatus status = 0;

    if( NULL == AudioUnitProcess_orig )
    {
        printf( "Not setup!\n" );
    }
    else {
        printf( "Calling original\n" );
        status = AudioUnitProcess_orig( inUnit, ioActionFlags, inTimeStamp, inNumberFrames, ioData );
    }

    return status;
}



// OSStatus (*AudioUnitProcess_orig) ( AudioUnit inUnit, 
// 			                        AudioUnitRenderActionFlags* __nullable ioActionFlags, 
// 									const AudioTimeStamp* inTimeStamp, 
// 									UInt32 inNumberFrames, 
// 									AudioBufferList* ioData );

// OSStatus (*AudioUnitProcess_hook) ( AudioUnit inUnit, 
// 			                        AudioUnitRenderActionFlags* __nullable ioActionFlags, 
// 									const AudioTimeStamp* inTimeStamp, 
// 									UInt32 inNumberFrames, 
// 									AudioBufferList* ioData );

#if defined(__LP64__)
    typedef struct mach_header_64 mach_header_t;
    typedef struct segment_command_64 segment_command_t;
    typedef struct section_64 section_t;
    typedef struct nlist_64 nlist_t;
    #define LC_SEGMENT_ARCH_DEPENDENT LC_SEGMENT_64
#else
    typedef struct mach_header mach_header_t;
    typedef struct segment_command segment_command_t;
    typedef struct section section_t;
    typedef struct nlist nlist_t;
    #define LC_SEGMENT_ARCH_DEPENDENT LC_SEGMENT
#endif

#define IN
#define OUT

static
void*
    iterate_indirect_symtab
    (
        IN      char*           symbol_name,
        IN      section_t*      section,
        IN      intptr_t        slide,
        IN      nlist_t*        symtab,
        IN      char*           strtab,
        IN      uint32_t*       indirect_symtab
    )
{
    const bool  is_data_const               = strcmp(section->segname, "__DATA_CONST") == 0;
    uint32_t*   indirect_symbol_indices     = indirect_symtab + section->reserved1;
    void**      indirect_symbol_bindings    = (void **)((uintptr_t)slide + section->addr);
    vm_prot_t   old_protection              = VM_PROT_READ;

    if( is_data_const )
    {
        printf( "is_data_const triggered\n" );
        mprotect( indirect_symbol_bindings, section->size, PROT_READ | PROT_WRITE );
    }

    for( uint64_t i = 0; i < section->size / sizeof( void* ); i++ )
    {
        printf( "Another one: %x\n", i );
        uint32_t symtab_index = indirect_symbol_indices[i];
        if( symtab_index == INDIRECT_SYMBOL_ABS ||
            symtab_index == INDIRECT_SYMBOL_LOCAL ||
            symtab_index == (INDIRECT_SYMBOL_LOCAL | INDIRECT_SYMBOL_ABS) )
        {
            continue;
        }
        uint32_t strtab_offset = symtab[symtab_index].n_un.n_strx;
        char *local_symbol_name = strtab + strtab_offset;
        bool symbol_name_longer_than_1 = symbol_name[0] && symbol_name[1];
        printf( "local_symbol_name: %s\n", local_symbol_name );
        if (strcmp(local_symbol_name, symbol_name) == 0)
        {
            printf( "match found\n" );
            return &indirect_symbol_bindings[i];
        }
        if (local_symbol_name[0] == '_')
        {
            if (strcmp(symbol_name, &local_symbol_name[1]) == 0)
            {
                printf( "match found part 2\n" );
                return &indirect_symbol_bindings[i];
            }
        }
    }

  if (is_data_const && 0) {
    int protection = 0;
    if (old_protection & VM_PROT_READ) {
      protection |= PROT_READ;
    }
    if (old_protection & VM_PROT_WRITE) {
      protection |= PROT_WRITE;
    }
    if (old_protection & VM_PROT_EXECUTE) {
      protection |= PROT_EXEC;
    }
    mprotect(indirect_symbol_bindings, section->size, protection);
  }
  return NULL;
}

void select_target_framework2( char* framework_image, char* symbol_name )
{
    uint32_t img_cnt = 0;
    uint32_t img_itr = 0;
    mach_header_t* mach_header = NULL;
    char* image_name = NULL;
    segment_command_t *curr_seg_cmd;
    segment_command_t *text_segment, *data_segment, *linkedit_segment;
    struct symtab_command *symtab_cmd = NULL;
    struct dysymtab_command *dysymtab_cmd = NULL;

    img_cnt = _dyld_image_count();

    for( img_itr = 0; img_itr < img_cnt; img_itr++ )
    {
        // printf(  "%x\n", img_itr );
        if( NULL == ( image_name = _dyld_get_image_name( img_itr ) ) )
            continue;

        // printf( "Image Name: %s\n", image_name );

        if( !strnstr( (const char*)image_name, (const char*)framework_image, PATH_MAX ) )
            continue;

        if( NULL == ( mach_header = _dyld_get_image_header( img_itr ) ) )
            continue;

        uintptr_t cur = (uintptr_t)mach_header + sizeof(mach_header_t);

        for( uint32_t i = 0; i < mach_header->ncmds; i++, cur += curr_seg_cmd->cmdsize )
        {
            curr_seg_cmd = (segment_command_t *)cur;
            if( curr_seg_cmd->cmd == LC_SEGMENT_ARCH_DEPENDENT )
            {
                if (strcmp(curr_seg_cmd->segname, "__LINKEDIT") == 0) {
                    linkedit_segment = curr_seg_cmd;
                } else if (strcmp(curr_seg_cmd->segname, "__DATA") == 0) {
                    data_segment = curr_seg_cmd;
                } else if (strcmp(curr_seg_cmd->segname, "__TEXT") == 0) {
                    text_segment = curr_seg_cmd;
                }
            } else if (curr_seg_cmd->cmd == LC_SYMTAB) {
                symtab_cmd = (struct symtab_command *)curr_seg_cmd;
            } else if (curr_seg_cmd->cmd == LC_DYSYMTAB) {
                dysymtab_cmd = (struct dysymtab_command *)curr_seg_cmd;
            }
        }

        if (!symtab_cmd || !linkedit_segment || !linkedit_segment) {
            // return NULL;
            return;
        }

        printf( "\n\n\nImage Name: %s\n", image_name );

        uintptr_t slide = (uintptr_t)mach_header - (uintptr_t)text_segment->vmaddr;
        uintptr_t linkedit_base = (uintptr_t)slide + linkedit_segment->vmaddr - linkedit_segment->fileoff;
        nlist_t *symtab = (nlist_t *)(linkedit_base + symtab_cmd->symoff);
        char *strtab = (char *)(linkedit_base + symtab_cmd->stroff);
        uint32_t symtab_count = symtab_cmd->nsyms;

        uint32_t *indirect_symtab = (uint32_t *)(linkedit_base + dysymtab_cmd->indirectsymoff);

        cur = (uintptr_t)mach_header + sizeof(mach_header_t);
        for (uint32_t i = 0; i < mach_header->ncmds; i++, cur += curr_seg_cmd->cmdsize) {
            curr_seg_cmd = (segment_command_t *)cur;
            printf( "an iter\n" );
            if (curr_seg_cmd->cmd == LC_SEGMENT_ARCH_DEPENDENT)
            {
                printf( "nsects: %x\n", curr_seg_cmd->nsects );

                // if (strcmp(curr_seg_cmd->segname, "__DATA") != 0 && strcmp(curr_seg_cmd->segname, "__DATA_CONST") != 0)
                // {
                //     continue;
                // }
                for (uint32_t j = 0; j < curr_seg_cmd->nsects; j++)
                {
                    section_t *sect = (section_t *)(cur + sizeof(segment_command_t)) + j;
                    void *stub = iterate_indirect_symtab(symbol_name, sect, slide, symtab, strtab, indirect_symtab);
                    if ((sect->flags & SECTION_TYPE) == S_LAZY_SYMBOL_POINTERS)
                    {
                        void *stub = iterate_indirect_symtab(symbol_name, sect, slide, symtab, strtab, indirect_symtab);
                        // if (stub)
                        //     return stub;
                    }
                    if ((sect->flags & SECTION_TYPE) == S_NON_LAZY_SYMBOL_POINTERS)
                    {
                        void *stub = iterate_indirect_symtab(symbol_name, sect, slide, symtab, strtab, indirect_symtab);
                        // if (stub)
                        //     return stub;
                    }
                }
            }
        }

    }
}


void
    select_target_framework
    (
        char* framework_image
    )
{
    uint32_t image_count = _dyld_image_count();
    uint32_t image_itr = 0;
    uint32_t cmd_itr = 0;
    struct mach_header* mach_header = NULL;
    struct load_command* lc = NULL;
    struct segment_command_64 *sc64 = NULL;
    struct segment_command *sc = NULL;
    char* image_name = NULL;

    // iterate images
    for( image_itr = 0; image_itr < image_count; image_itr++ )
    {
        image_name = _dyld_get_image_name( image_itr );
        if( NULL == image_name )
        {
            printf( "Failed to identify image. Skipping.\n" );
            continue;
        }

        printf( "Checking image: %s\n", image_name );
        continue;

        if( strnstr( (const char*)image_name, (const char*)framework_image, PATH_MAX ) )
        {
            mach_header = _dyld_get_image_header( image_itr );
            intptr_t slide = _dyld_get_image_vmaddr_slide( image_itr );

            // printf( "Checking image: %s\n", image_name );

            // Determine header type
            if( mach_header->magic == MH_MAGIC_64 )
            {
                printf( "64Bit mach load command\n" );
                lc = (struct load_command *)((unsigned char *)mach_header + sizeof(struct mach_header_64));
            }
            else
            {
                printf( "32Bit mach load command\n" );
                lc = (struct load_command *)((unsigned char *)mach_header + sizeof(struct mach_header));
            }

            // iterate commands
            for( cmd_itr = 0; cmd_itr < mach_header->ncmds; cmd_itr++ )
            {
                // printf( "cmd_itr: %x\n", cmd_itr );

                // handle segments
                if( lc->cmd == LC_SEGMENT_64 )
                {
                    sc64 = (struct segment_command_64 *)lc;
                    printf( "64Bit: %s (%llx - 0x%llx)\n", sc64->segname, sc64->vmaddr, sc64->vmsize );

                    struct section_64* sectStart = (struct section_64*)((char*)sc64 + sizeof( struct segment_command_64 ));
                    struct section_64* sectEnd = (struct section_64*)&sc64[sc64->nsects];
                    for( struct section_64* sect = sectStart; sect < sectEnd; ++sect )
                    {
                        if( strstr( sect->sectname, "__cstring" ) )
                        {
                            // char* wtf = (char*)(slide + sect->addr);
                            void* ptr = ptrauth_strip(slide + sect->addr, ptrauth_key_asia);
                            printf( "cstring? %s\n", (char*)ptr );
                            ptr = ptrauth_sign_unauthenticated( ptr, ptrauth_key_asia, 0 );
                            // wtf += ;
                            printf( "cstring? %s\n", (char*)ptr );
                        }

                        printf( "64Bit: sectname: %s, addr: %x, offset: %x, size: %x\n", sect->sectname, sect->addr, sect->offset, sect->size );
                    }
                }
                else if( lc->cmd == LC_SEGMENT )
                {
                    sc = (struct segment_command *)lc;
                    printf( "32Bit: %s (%x - 0x%x)\n", sc->segname, sc->vmaddr, sc->vmsize);

                    struct section* sectionStart = (struct section*)((char*)sc + sizeof( struct segment_command ));
                    struct section* sectionEnd = (struct section*)&sc[sc->nsects];
                    for( struct section* sect = sectionStart; sect < sectionEnd; ++sect )
                    {
                        printf( "32Bit: sectname: %s, addr: %x, offset: %x, size: %x\n", sect->sectname, sect->addr, sect->offset, sect->size );
                    }
                }


                // step commands
                lc = (struct load_command *)((unsigned char *)lc+lc->cmdsize);
            }
        }
    }
}


uint64_t getExecAddr(uint64_t addr, int index)
{
    const struct mach_header* header = _dyld_get_image_header(index);
    if (header == NULL){ return 0;}

    uint64_t libLoadAddr = (uint64_t)header;
    uint64_t exec_addr = libLoadAddr + addr;

    return exec_addr;
}


uint64_t getLibIndex( const char* que_image )
{
	int i = 0;
	int image_count = _dyld_image_count();

	for(; i < image_count; i++)
	{
		const char* req_image = _dyld_get_image_name(i);

		if(req_image && strcmp(req_image, que_image) == 0)
			{return i;}
	}
	return -1;
}




int main()
{
    // select_target_framework( "CoreAudio" );
    // select_target_framework2( "CoreAudio", "CoreAudioProcess" );


    // /System/Library/PrivateFrameworks/AudioToolboxCore.framework/Versions/A/AudioToolboxCore

    // void* audioUnitLib = NULL;
    
    // if( NULL == ( audioUnitLib = dlopen( "/System/Library/Frameworks/AudioUnit.framework/Versions/A/AudioUnit", RTLD_LAZY ) ) )
    // {
    //     printf( "Failed to open AudioUnit framework\n" );
    // }
    // else {
    //     if( NULL == ( AudioUnitProcess_orig = dlsym( audioUnitLib, "AudioUnitProcess" ) ) )
    //     {
    //         printf( "AudioUnitProcess symbol not found\n" );
    //     }
    //     else {
    //         printf( "Successfully linked AudioUnitProcess!\n" );
    //     }

    //     dlclose( audioUnitLib );
    // }

    // select_target_framework2( "AudioUnit", "CoreAudioProcess" );
    return 0;
}
