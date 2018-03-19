/***********************************************************************
 *                              _ooOoo_
 *                             o8888888o
 *                             88" . "88
 *                             (| -_- |)
 *                              O\ = /O
 *                          ____/`---'\____
 *                        .   ' \\| |// `.
 *                         / \\||| : |||// \
 *                       / _||||| -:- |||||- \
 *                         | | \\\ - /// | |
 *                       | \_| ''\---/'' | |
 *                        \ .-\__ `-` ___/-. /
 *                     ___`. .' /--.--\ `. . __
 *                  ."" '< `.___\_<|>_/___.' >'"".
 *                 | | : `- \`.;`\ _ /`;.`/ - ` : | |
 *                   \ \ `-. \_ __\ /__ _/ .-` / /
 *           ======`-.____`-.___\_____/___.-`____.-'======
 *                              `=---='
 *           .............................................
 *                    佛祖镇楼                  BUG辟易
 *            佛曰:
 *                    写字楼里写字间，写字间里程序员；
 *                    程序人员写程序，又拿程序换酒钱。
 *                    酒醒只在网上坐，酒醉还来网下眠；
 *                    酒醉酒醒日复日，网上网下年复年。
 *                    但愿老死电脑间，不愿鞠躬老板前；
 *                    奔驰宝马贵者趣，公交自行程序员。
 *                    别人笑我忒疯癫，我笑自己命太贱；
 *                    不见满街漂亮妹，哪个归得程序员？
 * ==========================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_smart.h
 *        Created:  2018年03月19日 14时11分33秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/

#ifndef _SPX_SMART_H_
#define _SPX_SMART_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define SpxAtomic volatile
#ifdef SpxDEBUG
#define ASSERT( s ) assert( s )
#define AssertLog( exp, fmt, ... )      \
    do {                                \
        if ( !exp ) {                   \
            printf( fmt, __VA_ARGS__ ); \
            assert( exp );              \
        }                               \
    } while ( 0 )
#else
#define ASSERT( s )
#define AssertLog( exp, fmt, ... ) printf( fmt, __VA_ARGS__ )
#endif

#ifdef SpxDEBUG
#define SpxInline
#else
#define SpxInline __attribute__( ( always_inline ) ) inline
#endif

#define SpxRightAlign( p, s ) ( ( (char *) p ) + ( s ) )
#define SpxLeftAlign( p, s ) ( ( (char *) p ) - ( s ) )
#define SpxMemAlign( d, a ) ( ( ( d ) + ( (a) -1 ) ) & ( ~( (a) -1 ) ) )

#if Spx32
#define SpxAlignSize 4
#elif Spx64
#define SpxAlignSize 8
#else
#define SpxAlignSize 8
#endif

#define SpxAlign( d ) SpxMemAlign( ( d ), SpxAlignSize )

/*
 * define the qualifier for smart pointer variable
 * if you declare or define a smart pointer,use like:
 *  SpxSmart int *ssp = NULL;
 * or like:
 *  SpxSmart struct people *p = spx_scoped_ptr(....);
 */
#define SpxSmart __attribute__( ( cleanup( spx_smart_cleanup ) ) )

/*
 * smart pointer style
 * normal : not the smart pointer,
 *          pointer but use the smart metadata for hiddle header
 * scoped : the scoped pointer,
 *          when the variable out of the life scope,destory it
 * shared : the shared pointer.
 *          destory itself just when 0 == ref_count.
 *          if the ref_count != 0 then variable out of the life scoped,
 *          the variable is not destory
 *          it usually use for return-value,mulit-ref...
 */
enum spx_smart_kind { normal, scoped, shared };

/*
 * the calback destructor for member in the smart pointer
 * /
typedef void( SpxSmartDestructorDelegate )( void *p );

/*
 * funtion wrap for alloc smart pointer
 * paras:
 *  kind : the smart pointer style
 *  size : the smart pointer memory size
 *  dtor : the destructor callback for smart pointer member
 *  err : errno
 * result:
 *  return the memory pointer as smart pointer
 */
void *spx_smart_ptr_wrap( const enum spx_smart_kind kind,
                          const int size,
                          SpxSmartDestructorDelegate *dtor,
                          int *err );

/*
 * incr the referenced count for shared pointer
 * paras :
 *  p : the smart pointer
 * result :
 *  return the smart pointer
 */
void *spx_smart_ref( void *p );

/*
 * cleanup callback for smart pointer
 * it's called auto by the variable out of the scope
 * paras:
 *  p : the smart pointer
 *
 */
SpxInline void spx_smart_cleanup( void *p );

/*
 * decr the referenced count fot smart pointer
 * the function use in the dtor for the member of smart pointer
 * when the decr the ref_count to 0,destory the smart pointer as member
 * paras:
 *  p : the smart pointer
 */
void spx_smart_unref( void *p );

/*
 * new a scoped pointer
 * the variable destory itself when out of the life-scoped
 * paras:
 *  size : the memory size
 *  dtor : the destructor callback for smart pointer's member
 *          if the scoped pointer have not member,it be NULL
 *  err : the errno pointer ,it's a out-argument
 * result:
 *  return the smart pointer
 */
#define spx_scoped_ptr( size, dtor, err ) \
    spx_smart_ptr_wrap( scoped, size, dtor, err )

/*
 * new a shared pointer
 * the variable destory itself when ref_count in the hiddle head to be 0
 * if you want to a return-value,member of struct,member of array,use it.
 * paras:
 *  size : the memory size
 *  dtor : the destructor callback for smart pointer's member
 *          if the shared pointer have not member,it be NULL
 *  err : the errno pointer ,it's a out-argument
 * result:
 *  return the smart pointer
 */
#define spx_shared_ptr( size, dtor, err ) \
    spx_smart_ptr_wrap( shared, size, dtor, err )

/*
 * new a normal pointer
 * the pointer can not destory auto
 * paras:
 *  size : the memory size
 *  dtor : the destructor callback for pointer's member
 *          if the pointer have not member,it be NULL
 *  err : the errno pointer ,it's a out-argument
 * result:
 *  return the smart pointer
 */
#define spx_normal_ptr( size, dtor, err ) \
    spx_smart_ptr_wrap( normal, size, dtor, err )

/*
 * return the smart pointer
 * when return the smart pointer,use statement like:
 *  return SpxSmartReturn(smart-pointer);
 */
#define SpxSmartReturn( p ) \
    ( {                     \
        spx_smart_ref( p ); \
        p;                  \
    } )

#ifdef __cplusplus
}
#endif
#endif
