/*************************************************************
 *                     _ooOoo_
 *                    o8888888o
 *                    88" . "88
 *                    (| -_- |)
 *                    O\  =  /O
 *                 ____/`---'\____
 *               .'  \\|     |//  `.
 *              /  \\|||  :  |||//  \
 *             /  _||||| -:- |||||-  \
 *             |   | \\\  -  /// |   |
 *             | \_|  ''\---/''  |   |
 *             \  .-\__  `-`  ___/-. /
 *           ___`. .'  /--.--\  `. . __
 *        ."" '<  `.___\_<|>_/___.'  >'"".
 *       | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *       \  \ `-.   \_ __\ /__ _/   .-` /  /
 *  ======`-.____`-.___\_____/___.-`____.-'======
 *                     `=---='
 *  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *           佛祖保佑       永无BUG
 *
 * ==========================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_smart.c
 *        Created:  2018年03月16日 22时57分44秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "spx_smart.h"

/*
 * structor for smart pointer
 */
struct spx_smart_ptr {
    struct spx_smart_metadata md;
    char p[ 0 ];
};

/*
 * print the metadata of smart pointer
 */
void spx_smart_print( const char *func, void *p ) { /*{{{*/
#ifdef SpxDEBUG
    struct spx_smart_metadata *md = spx_smart_get_metadata( p );
    printf( "func:%s,smart ptr:%p,metadata:kind=%d,bsize=%d,ref_count=%d.\n",
            func, p, md->kind, md->bsize, md->ref_count );
#endif
} /*}}}*/

void *spx_smart_ptr_wrap( const enum spx_smart_kind kind,
                          const int size,
                          SpxSmartDestructorDelegate *dtor,
                          int *err ) { /*{{{*/
    int hlen = sizeof( struct spx_smart_ptr );
    int blen = SpxAlign( size );
    struct spx_smart_ptr *p
        = (struct spx_smart_ptr *) calloc( sizeof( char ), hlen + blen );
    ASSERT( p );
    struct spx_smart_metadata *md = (struct spx_smart_metadata *) p;
    md->kind = kind;
    md->dtor = dtor;
    md->bsize = blen;
    md->freesize = blen - size;
    md->ref_count = 1;
    return p->p;
} /*}}}*/

void *spx_smart_ref( void *p ) { /*{{{*/
    struct spx_smart_metadata *md = spx_smart_get_metadata( p );
    if ( normal == md->kind || scoped == md->kind ) {
        ASSERT( shared == md->kind );
    }
    if ( shared == md->kind ) {
        md->ref_count++;
    }
    spx_smart_print( __FUNCTION__, p );
    return p;
} /*}}}*/

SpxInline void spx_smart_cleanup( void *p ) { /*{{{*/
    void *ptr = *(void **) p;
    if ( NULL == p || NULL == ptr )
        return;
    struct spx_smart_metadata *md = spx_smart_get_metadata( ptr );
    spx_smart_print( __FUNCTION__, ptr );
    switch ( md->kind ) {
        case ( normal ):
        case ( scoped ): {
            if ( NULL != md->dtor ) {
                md->dtor( &ptr );
            } else {
                free( md );
                *(void **) p = NULL;
            }
            break;
        }
        case ( shared ): {
            if ( 1 == ( md->ref_count-- ) ) {
                if ( NULL != md->dtor ) {
                    md->dtor( &ptr );
                } else {
                    free( md );
                    *(void **) p = NULL;
                }
            }
            break;
        }
    }
} /*}}}*/

void spx_smart_unref( void *p ) { /*{{{*/
    struct spx_smart_metadata *md = spx_smart_get_metadata( p );
    spx_smart_print( __FUNCTION__, p );
    switch ( md->kind ) {
        case ( normal ):
        case ( scoped ): {
            if ( NULL != md->dtor ) {
                md->dtor( &p );
            } else {
                free( md );
                *(void **) p = NULL;
            }
            break;
        }
        case ( shared ): {
            if ( 0 == ( --md->ref_count ) ) {
                if ( NULL != md->dtor ) {
                    md->dtor( &p );
                } else {
                    free( md );
                    *(void **) p = NULL;
                }
            }
            break;
        }
    }
} /*}}}*/

SpxInline void spx_smart_ptr_free( void *p ) { /*{{{*/
    union {
        void **rptr;
        void *ptr;
    } c = { .ptr = p };
    struct spx_smart_metadata *md = spx_smart_get_metadata( p );
    free( md );
    md = NULL;
    *( c.rptr ) = NULL;
} /*}}}*/

SpxInline struct spx_smart_metadata *spx_smart_get_metadata(
    const void *p ) { /*{{{*/
    int hlen = sizeof( struct spx_smart_metadata );
    struct spx_smart_metadata *md
        = (struct spx_smart_metadata *) SpxLeftAlign( p, hlen );
    return md;
} /*}}}*/

void *spx_smart_ptr_resize( void *p,
                            const int size,
                            const bool_t is_mulit_ref_error,
                            int *err ) { /*{{{*/
    struct spx_smart_metadata *md = spx_smart_get_metadata( p );
    if ( size <= md->bsize ) {
        return p;
    }
    int usize = md->bsize - md->freesize;
    int blen = SpxAlign( size );
    int hlen = sizeof( struct spx_smart_metadata );
    if ( scoped == md->kind || normal == md->kind ) {
        struct spx_smart_ptr *ptr
            = realloc( (struct spx_smart_ptr *) md, hlen + blen );
        ASSERT( ptr );
        struct spx_smart_metadata *md_new = (struct spx_smart_metadata *) ptr;
        md_new->bsize = blen;
        md_new->freesize = blen - usize;
        return ptr->p;
    }
    if ( shared == md->kind ) {
        if ( ( 1 == md->ref_count ) && is_mulit_ref_error ) {
            *err = EPERM;
            return NULL;
        }
        struct spx_smart_ptr *ptr
            = realloc( (struct spx_smart_ptr *) md, hlen + blen );
        ASSERT( ptr );
        struct spx_smart_metadata *md_new = (struct spx_smart_metadata *) ptr;
        md_new->bsize = blen;
        md_new->freesize = blen - usize;
        return ptr->p;
    }
    return NULL;
} /*}}}*/

SpxInline void spx_smart_update_freesize( const void *p,
                                          const int usize ) { /*{{{*/
    struct spx_smart_metadata *md = spx_smart_get_metadata( p );
    md->freesize = md->bsize - usize;
} /*}}}*/
