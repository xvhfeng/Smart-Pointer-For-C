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
 *       Filename:  spx_smart_test.c
 *        Created:  2018年03月19日 16时21分58秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <check.h>
#include "spx_smart.h"

START_TEST( spx_smart_test_nocrash ) {
    do {
        SpxSmart int *tp = NULL;
    } while ( 0 );
    int ok = 0;
}
END_TEST

START_TEST( spx_smart_test_destory ) {
    int err = 0;
    do {
        SpxSmart int *tp = spx_scoped_ptr( sizeof( int ), NULL, &err );
        ck_assert_msg( NULL != tp, "can not alloc the scoped pointer." );
        *tp = 9;
    } while ( 0 );
}
END_TEST

int *spx_smart_return_add_refcount( ) {
    int err = 0;
    SpxSmart int *tp = spx_shared_ptr( sizeof( int ), NULL, &err );
    ck_assert_msg( NULL != tp, "can not alloc the scoped pointer." );
    *tp = 9;
    return SpxSmartReturn( tp );
}

int *spx_smart_return_not_add_refcount( ) {
    int err = 0;
    SpxSmart int *tp = spx_shared_ptr( sizeof( int ), NULL, &err );
    ck_assert_msg( NULL != tp, "can not alloc the scoped pointer." );
    *tp = 20;
    return tp;
}

START_TEST( spx_smart_test_return_add_refcount ) {
    do {
        SpxSmart int *tp = spx_smart_return_add_refcount( );
        ck_assert_msg( tp, "return smart pointer is null." );
    } while ( 0 );
}
END_TEST

START_TEST( spx_smart_test_return_not_add_refcount ) {
    SpxSmart int *tp = spx_smart_return_not_add_refcount( );
    ck_assert_msg( tp, "return smart with no add refcount." );
}
END_TEST

struct spx_smart_people {
    int age;
    int *count;
};
void spx_smart_dtor( void **p ) {
    struct spx_smart_people *peo = (struct spx_smart_people *) *p;
    if ( NULL != peo->count ) {
        spx_smart_unref( peo->count );
    }
    spx_smart_ptr_free( peo );
}

START_TEST( spx_smart_test_member ) {
    int err = 0;
    SpxSmart struct spx_smart_people *p
        = spx_scoped_ptr( sizeof( *p ), spx_smart_dtor, &err );
    ck_assert_msg( p, "not alloc people." );
    p->age = 10;

    SpxSmart int *count = spx_shared_ptr( sizeof( int ), NULL, &err );
    ck_assert_msg( count, "not alloc count." );
    *count = 100;
    p->count = spx_smart_ref( count );
}
END_TEST

int *spx_smart_return_null( ) {
    int err = 0;
    SpxSmart int *t = spx_shared_ptr( sizeof( int ), NULL, &err );
    ck_assert_msg( t, "not alloc int." );
    *t = 0;
    return t;
}

START_TEST( spx_smart_test_null_pointer ) {
    SpxSmart int *t = spx_smart_return_null( );
    ck_assert_msg( !t, "return not null pointer,%p,%p.", t, &t );
}
END_TEST

Suite *make_spx_smart_test( void ) {
    Suite *s = suite_create( "spx_smart" );
    TCase *tc = tcase_create( "spx_smart" );
    suite_add_tcase( s, tc );
    tcase_add_test( tc, spx_smart_test_nocrash );
    tcase_add_test( tc, spx_smart_test_destory );
    tcase_add_test( tc, spx_smart_test_return_add_refcount );
    tcase_add_test( tc, spx_smart_test_return_not_add_refcount );
    tcase_add_test( tc, spx_smart_test_member );
    tcase_add_test( tc, spx_smart_test_null_pointer );
    return s;
}
