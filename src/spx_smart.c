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

#include <stdio.h>
#include <stdlib.h>

#include "spx_smart.h"

/*
 * the metadata for smart pointer
 * it's also as the hiddle head for smart pointer
 */
struct spx_smart_metadata {
    int kind;
    SpxAtomic int ref_count;
    int msize;  // memory size
    int freesize;
    SpxSmartDestructorDelegate *dtor;
};

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
    int hlen = sizeof( struct spx_smart_metadata );
    struct spx_smart_metadata *md
        = (struct spx_smart_metadata *) SpxLeftAlign( p, hlen );
    printf( "func:%s,smart ptr:%lld,metadata:kind=%d,msize=%d,ref_count=%d.\n",
            func, p, md->kind, md->msize, md->ref_count );
} /*}}}*/

void *spx_smart_ptr_wrap( const enum spx_smart_kind kind,
                          const int size,
                          SpxSmartDestructorDelegate *dtor,
                          int *err ) { /*{{{*/
    int hlen = sizeof( struct spx_smart_ptr );
    int blen = SpxAlign( size );
    struct spx_smart_ptr *p
        = (struct spx_smart_ptr *) calloc( sizeof( char ), hlen + blen );
    if ( NULL == p ) {
        ASSERT( p );
    }
    struct spx_smart_metadata *md = (struct spx_smart_metadata *) p;
    md->kind = kind;
    md->dtor = dtor;
    md->msize = hlen + blen;
    md->ref_count = 1;
    return p->p;
} /*}}}*/

void *spx_smart_ref( void *p ) { /*{{{*/
    int hlen = sizeof( struct spx_smart_metadata );
    struct spx_smart_metadata *md
        = (struct spx_smart_metadata *) SpxLeftAlign( p, hlen );
    if ( normal == md->kind || scoped == md->kind ) {
        ASSERT( scoped == md->kind );
    }
    if ( shared == md->kind ) {
        md->ref_count++;
        printf( "ref ptr=%lld,current ref_count=%d\n", p, md->ref_count );
    }
    spx_smart_print( __FUNCTION__, p );
    return p;
} /*}}}*/

SpxInline void spx_smart_cleanup( void *p ) { /*{{{*/
    void *ptr = *(void **) p;
    if ( NULL == p )
        return;
    union {
        void **rptr;
        void *ptr;
    } c;
    c.ptr = ptr;
    int hlen = sizeof( struct spx_smart_metadata );
    struct spx_smart_metadata *md
        = (struct spx_smart_metadata *) SpxLeftAlign( ptr, hlen );
    spx_smart_print( __FUNCTION__, ptr );
    switch ( md->kind ) {
        case ( normal ):
        case ( scoped ): {
            if ( NULL != md->dtor ) {
                md->dtor( ptr );
            }
            struct spx_smart_ptr *smart = (struct spx_smart_ptr *) md;
            free( smart );
            *(void **) p = NULL;
            break;
        }
        case ( shared ): {
            if ( 1 == ( md->ref_count-- ) ) {
                printf( "shared over = %lld\n", ptr );
                if ( NULL != md->dtor ) {
                    md->dtor( ptr );
                }
                free( md );
                *(void **) p = NULL;
            } else {
                printf( "shared ptr:%lld,unref count:%d.\n", ptr,
                        md->ref_count );
            }
            break;
        }
    }
} /*}}}*/

void spx_smart_unref( void *p ) { /*{{{*/
    int hlen = sizeof( struct spx_smart_metadata );
    struct spx_smart_metadata *md
        = (struct spx_smart_metadata *) SpxLeftAlign( p, hlen );
    spx_smart_print( __FUNCTION__, p );
    switch ( md->kind ) {
        case ( normal ):
        case ( scoped ): {
            if ( NULL != md->dtor ) {
                md->dtor( p );
            }
            struct spx_smart_ptr *smart = (struct spx_smart_ptr *) md;
            free( smart );
            *(void **) p = NULL;
            break;
        }
        case ( shared ): {
            if ( 0 == ( --md->ref_count ) ) {
                printf( "destory shared ptr:%lld,unref count:%d.\n", p,
                        md->ref_count );
                if ( NULL != md->dtor ) {
                    md->dtor( p );
                }
                free( md );
                *(void **) p = NULL;
            } else {
                printf( "shared ptr:%lld,unref count:%d.\n", p, md->ref_count );
            }
            break;
        }
    }
} /*}}}*/

struct spx_people_test {
    int age;
    int *count;
};

void spx_smart_people_cleanup( void *p ) { /*{{{*/
    struct spx_people_test *t = (struct spx_people_test *) p;
    if ( NULL != t->count ) {
        spx_smart_print( __FUNCTION__, t->count );
        spx_smart_unref( t->count );
    }
} /*}}}*/

int main( int argc, char **argv ) {
    int err = 0;
    do {
        SpxSmart struct spx_people_test *spt
            = spx_shared_ptr( sizeof( *spt ), spx_smart_people_cleanup, &err );

        ASSERT( spt );
        spx_smart_print( __FUNCTION__, spt );

        SpxSmart int *t = spx_shared_ptr( sizeof( int ), NULL, &err );
        ASSERT( spt );
        spx_smart_print( __FUNCTION__, t );
        spt->age = 2;
        spt->count = spx_smart_ref( t );

        goto r1;
        SpxSmart struct spx_people_test *spt2
            = spx_shared_ptr( sizeof( *spt2 ), spx_smart_people_cleanup, &err );

        ASSERT( spt2 );
        spx_smart_print( __FUNCTION__, spt2 );

    } while ( 0 );
    printf( "pause\n" );
r1:
    printf( "over \n" );
    return 0;
}
