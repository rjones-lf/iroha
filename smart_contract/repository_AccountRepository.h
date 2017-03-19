/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class repository_AccountRepository */

#ifndef _Included_repository_AccountRepository
#define _Included_repository_AccountRepository
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     repository_AccountRepository
 * Method:    updateQuantity
 * Signature: (Ljava/lang/String;Ljava/lang/String;J)V
 */
JNIEXPORT void JNICALL
Java_repository_AccountRepository_updateQuantity
(JNIEnv * , jclass , jstring , jstring , jlong ) ;

/*
 * Class:     repository_AccountRepository
 * Method:    attach
 * Signature: (Ljava/lang/String;Ljava/lang/String;J)V
 */
JNIEXPORT void JNICALL
Java_repository_AccountRepository_attach
(JNIEnv * , jclass , jstring , jstring , jlong ) ;

/*
 * Class:     repository_AccountRepository
 * Method:    findByUuid
 * Signature: (Ljava/lang/String;)Ljava/util/HashMap;
 */
JNIEXPORT jobject
JNICALL Java_repository_AccountRepository_findByUuid
    (JNIEnv * , jclass, jstring);

/*
 * Class:     repository_AccountRepository
 * Method:    add
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_repository_AccountRepository_add
(JNIEnv * , jclass , jstring , jstring ) ;

#ifdef __cplusplus
}
#endif
#endif
