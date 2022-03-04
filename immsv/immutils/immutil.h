/*       OpenSAF
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Ericsson AB
 *
 */

#ifndef OPENSAF_OSAF_LIBS_COMMON_IMMSV_INCLUDE_IMMUTIL_H_
#define OPENSAF_OSAF_LIBS_COMMON_IMMSV_INCLUDE_IMMUTIL_H_
/**
 * @file immutil.h
 * @brief Utilities for SAF-IMM users.
 *
 *    This is a collection of utility functions intended to make life easier for
 *    programmers forced to use the SAF-IMM.
 */

/*
 *  $copyright$
 */

#include <saImmOm.h>
#include <saImmOi.h>
#include <sys/types.h>
#include <regex.h>
#include <limits.h>
#include <stdbool.h>

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#endif

/**
 * @defgroup CcbUtils Defines an interface for simplified handling of CCBs
 *
 * The configuration module in SAF, the IMM, uses a kind of transactions for
 * configuration sequences. A CCB may contain many configuration operations and
 * may be aborted at any time.  This forces an application to check and store
 * all configuration operations for an CCB and execute them on a "CCB Apply" or
 * discard the operations on a "CCB Abort"
 *
 * This is a collection of functions for storing the configuration operations
 * for an CCB.
 */
/*@{*/

/**
 * Defines the type of the function called when a fatal error occurs.
 */
typedef void (*ImmutilErrorFnT)(char const *fmt, ...)
    __attribute__((format(printf, 1, 2)));

/**
 * Defines the command operation types.
 */
enum CcbUtilOperationType { CCBUTIL_CREATE, CCBUTIL_DELETE, CCBUTIL_MODIFY };

/**
 * This structure holds a stored command operation. The stored data is the same
 * as the one passed in the operation call-back functions.
 */
typedef struct CcbUtilOperationData {
  struct CcbUtilOperationData *next;
  void *userData;
  SaAisErrorT userStatus;
  enum CcbUtilOperationType operationType;
  SaNameT objectName;
  SaImmOiCcbIdT ccbId;
  union {
    struct {
      SaImmClassNameT className;
      const SaNameT *parentName;
      const SaImmAttrValuesT_2 **attrValues;
    } create;
    struct {
      const SaNameT *objectName;  // redundant, can be removed
    } delete_;
    struct {
      const SaNameT *objectName;  // redundant, can be removed
      const SaImmAttrModificationT_2 **attrMods;
    } modify;
  } param;
} CcbUtilOperationData_t;

/**
 * A CCB object, holds the stored operations for a CCB.
 */
typedef struct CcbUtilCcbData {
  struct CcbUtilCcbData *next;
  SaImmOiCcbIdT ccbId;
  void *userData;
  void *memref;
  struct CcbUtilOperationData *operationListHead;
  struct CcbUtilOperationData *operationListTail;
} CcbUtilCcbData_t;

/**
 * Find a CCB object.
 * @param id The CCB identity tag.
 * @return The CCB object or NULL if not found.
 */
EXTERN_C struct CcbUtilCcbData *ccbutil_findCcbData(SaImmOiCcbIdT id);

/**
 * Get a CCB object. The object is created with #ccbutil_createCcbData if it
 * doesn't exist.
 * @param id The CCB identity tag.
 * @return The CCB object.
 */
EXTERN_C struct CcbUtilCcbData *ccbutil_getCcbData(SaImmOiCcbIdT id);

/**
 * Find if there is any ccb op going on.
 * @param id : None.
 * @return : true if no ccb op undergoing else false.
 */
EXTERN_C bool ccbutil_EmptyCcbExists();

/**
 * Delete a CCB object. All memory associated with the CCB is freed.
 * @param id The CCB identity tag.
 */
EXTERN_C void ccbutil_deleteCcbData(struct CcbUtilCcbData *ccb);

/**
 * Add a Create operation to a CCB data object.
 */
EXTERN_C CcbUtilOperationData_t *ccbutil_ccbAddCreateOperation(
    struct CcbUtilCcbData *ccb, const SaImmClassNameT className,
    const SaNameT *parentName, const SaImmAttrValuesT_2 **attrValues);

/**
 * Add a Create operation to a CCB data object including objectName.
 */
EXTERN_C CcbUtilOperationData_t *ccbutil_ccbAddCreateOperation_2(
    struct CcbUtilCcbData *ccb, const SaNameT *objectName,
    const SaImmClassNameT className, const SaNameT *parentName,
    const SaImmAttrValuesT_2 **attrValues);

/**
 * Add a Delete operation to a CCB data object.
 */
EXTERN_C void ccbutil_ccbAddDeleteOperation(struct CcbUtilCcbData *ccb,
                                            const SaNameT *objectName);

/**
 * Add a Modify operation to a CCB data object. Modify of an object that is
 * beeing created in the same CCB is not allowed.
 * @param ccb The owning CCB object
 * @param objectName DN
 * @param attrMods modifications
 * @return 0 if OK, -1 otherwise
 */
EXTERN_C int ccbutil_ccbAddModifyOperation(
    CcbUtilCcbData_t *ccb, const SaNameT *objectName,
    const SaImmAttrModificationT_2 **attrMods);

EXTERN_C CcbUtilOperationData_t *ccbutil_getNextCcbOp(
    SaImmOiCcbIdT id, CcbUtilOperationData_t *opData);

/**
 * Find a CCB operation using DN
 * @param id
 * @param dn
 *
 * @return CcbUtilOperationData_t*
 */
EXTERN_C CcbUtilOperationData_t *ccbutil_getCcbOpDataByDN(SaImmOiCcbIdT id,
                                                          const SaNameT *dn);

/*@}*/
/**
 * @defgroup ImmHelpUtils General help utilities for IMM users
 *
 * Some utility functions that seems to be generic and useful enough to be
 * exported in a common library.
 */
/*@{*/

/**
 * Duplicates a string. The memory is owned by the passed CCB object and will be
 * freed automatically when this object is deleted.
 * @param ccb The owning CCB object
 * @param source The source string
 * @return A string duplicate. will be freed automatically when owning CCB
 *      object is deleted.
 */
EXTERN_C char *immutil_strdup(struct CcbUtilCcbData *ccb, char const *source);

/**
 * Get the ClassName for an object. Common usage is to get the class
 * of the parent on a CreateOperation.
 */
EXTERN_C char const *immutil_getClassName(struct CcbUtilCcbData *ccb,
                                          SaImmHandleT immHandle,
                                          const SaNameT *objectName);

/**
 * Searches a distinguished name for a numeric value identified by the passed
 * key.  If the key is not found or the value is not numeric LONG_MIN is
 * returned. The value may be decimal, hex or octal as specified by the
 * "strtol()" C-function. The value may be terminated by a ',' character or
 * end-of-name.
 * @param key The key. Must be appended with a '=', like "proto=".
 * @param name The distinguished name to scan.
 * @return The numeric value or LONG_MIN in case of error.
 */
EXTERN_C long immutil_getNumericValue(char const *key, SaNameT const *name);

/**
 * Searches a distinguished name for a string value identified by the passed
 * key. If the key is not found or the value empty NULL is returned. The value
 * may be terminated by a ',' character or end-of-name.
 * @param key The key. Must be appended with a '=', like "proto=".
 * @param name The distinguished name to scan.
 * @return The string value (statically allocated) or NULL in case of error.
 */
EXTERN_C char const *immutil_getStringValue(char const *key,
                                            SaNameT const *name);

/**
 * Get the n'th item in a disinguished name as a string.
 * @param dn The disinguished name
 * @param index The index of the item, 0 is the first.
 * @return The item as a (statically allocated) string or NULL if not found.
 */
EXTERN_C char const *immutil_getDnItem(SaNameT const *dn, unsigned int index);

/**
 * Scans the attributes for an an attribute identified by the passed name. If
 * the attribute is found the value with the passed index is returned.
 * @param attr The attribute vector
 * @param name The name of the attribute
 * @param index The value index (always 0 for single-value attributes)
 * @return The value or NULL if not found
 */
EXTERN_C const SaNameT *immutil_getNameAttr(const SaImmAttrValuesT_2 **attr,
                                            char const *name,
                                            unsigned int index);

/**
 * Scans the attributes for an an attribute identified by the passed name. If
 * the attribute is found the value with the passed index is returned.
 * @param attr The attribute vector
 * @param name The name of the attribute
 * @param index The value index (always 0 for single-value attributes)
 * @return The value or NULL if not found
 */
EXTERN_C char const *immutil_getStringAttr(const SaImmAttrValuesT_2 **attr,
                                           char const *name,
                                           unsigned int index);

/**
 * Scans the attributes for an an attribute identified by the passed name. If
 * the attribute is found the value with the passed index is returned.
 * @param attr The attribute vector
 * @param name The name of the attribute
 * @param index The value index (always 0 for single-value attributes)
 * @return A pointer to the value or NULL if not found
 */
EXTERN_C const SaUint32T *immutil_getUint32Attr(const SaImmAttrValuesT_2 **attr,
                                                char const *name,
                                                unsigned int index);

/**
 * Return how many values a named attribute has
 * @param attrName
 * @param attr
 * @param attrValuesNumber
 *
 * @return SaAisErrorT
 */
EXTERN_C SaAisErrorT immutil_getAttrValuesNumber(
    const char *attrName, const SaImmAttrValuesT_2 **attr,
    SaUint32T *attrValuesNumber);

/**
 * Scans the attributes for an an attribute identified by the passed name. If
 * the attribute is found the value with the passed index is returned.
 * @param attr The attribute vector
 * @param name The name of the attribute
 * @param index The value index (always 0 for single-value attributes)
 * @return A pointer to the value or NULL if not found
 */
EXTERN_C const SaTimeT *immutil_getTimeAttr(const SaImmAttrValuesT_2 **attr,
                                            char const *name,
                                            unsigned int index);

/**
 * Return the value for a named attribute from the supplied attribute
 * array. Convert the returned value to the specified type and store in the
 * supplied parameter.
 *
 * @param attrName name of attribute for which the value should be returned
 * @param attr attribute array as returned from e.g.  saImmOmSearchNext
 * @param index array index of the value to be returned (support for multi value
 *              attributes)
 * @param param [out] attribute value stored here
 *
 * @return SaAisErrorT SA_AIS_OK when conversion was successful
 */
EXTERN_C SaAisErrorT immutil_getAttr(const char *attrName,
                                     const SaImmAttrValuesT_2 **attr,
                                     SaUint32T index, void *param);

/**
 * Works as "strchr()" but with a length limit. It is provided here since
 * "strnchr()" is not standard, and is useful when scanning SaNameT.
 */
EXTERN_C char const *immutil_strnchr(char const *str, int c, size_t length);

/**
 * Match a name against a regular expression. The "regexec" POSIX function is
 * used. The pre-copmpiled re must be built with "REG_NOSUB" flag and you should
 * really use "REG_EXTENDED|REG_NOSUB". Example (error checks omitted);
 * \code
 *    regex_t preg;
 *    regcomp(&preg, "key=(none|[0-9]+), REG_EXTENDED|REG_NOSUB);
 *    if (immutil_matchName(name, &preg) == 0) {
 *       ...
 *    }
 *    regfree(&preg);
 * \endcode
 * @param re The regular expression
 * @param preg A pre-compiled regular expression (regcomp)
 * @return Return-value from the regexec function (0 if match)
 */
EXTERN_C int immutil_matchName(SaNameT const *name, regex_t const *preg);

/**
 * Update one runtime attribute of the specified object.
 *
 * @param immOiHandle
 * @param dn
 * @param attributeName
 * @param attrValueType
 * @param value
 *
 * @return SaAisErrorT
 */
/***/
EXTERN_C SaAisErrorT immutil_update_one_rattr(SaImmOiHandleT immOiHandle,
                                              const char *dn,
                                              SaImmAttrNameT attributeName,
                                              SaImmValueTypeT attrValueType,
                                              void *value);

/**
 * Get class name from object name.
 * @param objectName
 *
 * @return SaImmClassNameT
 */
EXTERN_C SaImmClassNameT immutil_get_className(const SaNameT *objectName);

/**
 * Get value type of named attribute for an object. The caller must free the
 * memory returned.
 *
 * @param objectName
 * @param attrName
 * @param attrValueType
 *
 * @return SaAisErrorT
 */
EXTERN_C SaAisErrorT immutil_get_attrValueType(const SaImmClassNameT className,
                                               SaImmAttrNameT attrName,
                                               SaImmValueTypeT *attrValueType);

/**
 * Create attribute value from type and string. The caller must free the memory
 * returned.
 * @param attrValueType
 * @param str
 *
 * @return void*
 */
EXTERN_C void *immutil_new_attrValue(SaImmValueTypeT attrValueType,
                                     const char *str);

/*@}*/

/**
 * @defgroup ImmCallWrappers IMM call wrappers
 *
 * This wrapper interface offloads the burden to handle return values and
 * retries for each and every IMM-call. It makes the code cleaner.
 */
/*@{*/

/**
 * Controls the behavior of the wrapper functions.
 */
struct ImmutilWrapperProfile {
  int errorsAreFatal;
  /**< If set the calling program is aborted on an error return from SAF.  */
  unsigned int nTries;
  /**< Number of tries before we give up.  */
  unsigned int retryInterval;
  /**< Interval in milli-seconds between tries.  */
};

/**
 * Default: errorsAreFatal=1, nTries=5, retryInterval=400.
 */
EXTERN_C SaAisErrorT immutil_saImmOiInitialize_2(
    SaImmOiHandleT *immOiHandle, const SaImmOiCallbacksT_2 *immOiCallbacks,
    const SaVersionT *version);

EXTERN_C SaAisErrorT immutil_saImmOiInitialize_o3(
    SaImmOiHandleT *immOiHandle, const SaImmOiCallbacksT_o3 *immOiCallbacks,
    const SaVersionT *version);

EXTERN_C SaAisErrorT immutil_saImmOiSelectionObjectGet(
    SaImmOiHandleT immOiHandle, SaSelectionObjectT *selectionObject);

EXTERN_C SaAisErrorT immutil_saImmOiClassImplementerSet(
    SaImmOiHandleT immOiHandle, const char *className);

EXTERN_C SaAisErrorT immutil_saImmOiClassImplementerRelease(
    SaImmOiHandleT immOiHandle, const char *className);

EXTERN_C SaAisErrorT immutil_saImmOiObjectImplementerSet(
    SaImmOiHandleT immOiHandle, const SaNameT *objectName, SaImmScopeT scope);

EXTERN_C SaAisErrorT immutil_saImmOiObjectImplementerSet_o3(
    SaImmOiHandleT immOiHandle, const char *objectName, SaImmScopeT scope);

EXTERN_C SaAisErrorT immutil_saImmOiObjectImplementerRelease(
    SaImmOiHandleT immOiHandle, const SaNameT *objectName, SaImmScopeT scope);

EXTERN_C SaAisErrorT immutil_saImmOiObjectImplementerRelease_o3(
    SaImmOiHandleT immOiHandle, const char *objectName, SaImmScopeT scope);

EXTERN_C SaAisErrorT immutil_saImmOiImplementerSet(
    SaImmOiHandleT immOiHandle, const SaImmOiImplementerNameT implementerName);

EXTERN_C SaAisErrorT
immutil_saImmOiImplementerClear(SaImmOiHandleT immOiHandle);

EXTERN_C SaAisErrorT immutil_saImmOiRtObjectCreate_2(
    SaImmOiHandleT immOiHandle, const SaImmClassNameT className,
    const SaNameT *parentName, const SaImmAttrValuesT_2 **attrValues);

EXTERN_C SaAisErrorT immutil_saImmOiRtObjectCreate_o2(
    SaImmOiHandleT immOiHandle, const SaImmClassNameT className,
    const char *parentName, const SaImmAttrValuesT_2 **attrValues);

EXTERN_C SaAisErrorT immutil_saImmOiRtObjectCreate_o3(
    SaImmOiHandleT immOiHandle, const SaImmClassNameT className,
    const char *objectName, const SaImmAttrValuesT_2 **attrValues);

EXTERN_C SaAisErrorT immutil_saImmOiRtObjectDelete(SaImmOiHandleT immOiHandle,
                                                   const SaNameT *objectName);

EXTERN_C SaAisErrorT immutil_saImmOiRtObjectDelete_o2(
    SaImmOiHandleT immOiHandle, const char *objectName);

EXTERN_C SaAisErrorT immutil_saImmOiRtObjectDelete_o3(
    SaImmOiHandleT immOiHandle, const char *objectName);

EXTERN_C SaAisErrorT immutil_saImmOiRtObjectUpdate_2(
    SaImmOiHandleT immOiHandle, const SaNameT *objectName,
    const SaImmAttrModificationT_2 **attrMods);

EXTERN_C SaAisErrorT immutil_saImmOiRtObjectUpdate_o2(
    SaImmOiHandleT immOiHandle, const char *objectName,
    const SaImmAttrModificationT_2 **attrMods);

EXTERN_C SaAisErrorT immutil_saImmOiRtObjectUpdate_o3(
    SaImmOiHandleT immOiHandle, const char *objectName,
    const SaImmAttrModificationT_2 **attrMods);

EXTERN_C SaAisErrorT immutil_saImmOiAdminOperationResult(
    SaImmOiHandleT immOiHandle, SaInvocationT invocation, SaAisErrorT result);

EXTERN_C SaAisErrorT immutil_saImmOiAdminOperationResult_o2(
    SaImmOiHandleT immOiHandle, SaInvocationT invocation, SaAisErrorT result,
    const SaImmAdminOperationParamsT_2 **returnParams);

EXTERN_C SaAisErrorT immutil_saImmOiAugmentCcbInitialize(
    SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId64,
    SaImmCcbHandleT *ccbHandle, SaImmAdminOwnerHandleT *ownerHandle);

EXTERN_C SaAisErrorT immutil_saImmOmInitialize(
    SaImmHandleT *immHandle, const SaImmCallbacksT *immCallbacks,
    const SaVersionT *version);

EXTERN_C SaAisErrorT immutil_saImmOmInitialize_o2(
    SaImmHandleT *immHandle, const SaImmCallbacksT_o2 *immCallbacks,
    SaVersionT *version);

EXTERN_C SaAisErrorT immutil_saImmOmFinalize(SaImmHandleT immHandle);

EXTERN_C SaAisErrorT immutil_saImmOmSelectionObjectGet(
    SaImmHandleT immHandle, SaSelectionObjectT *selectionObject);

EXTERN_C SaAisErrorT immutil_saImmOmAccessorInitialize(
    SaImmHandleT immHandle, SaImmAccessorHandleT *accessorHandle);

EXTERN_C SaAisErrorT immutil_saImmOmAccessorGet_2(
    SaImmAccessorHandleT accessorHandle, const SaNameT *objectName,
    const SaImmAttrNameT *attributeNames, SaImmAttrValuesT_2 ***attributes);

EXTERN_C SaAisErrorT immutil_saImmOmAccessorGet_o2(
    SaImmAccessorHandleT accessorHandle, const char *objectName,
    const SaImmAttrNameT *attributeNames, SaImmAttrValuesT_2 ***attributes);

EXTERN_C SaAisErrorT immutil_saImmOmAccessorGet_o3(
    SaImmAccessorHandleT accessorHandle, const char *objectName,
    const SaImmAttrNameT *attributeNames, SaImmAttrValuesT_2 ***attributes);

EXTERN_C SaAisErrorT immutil_saImmOmAccessorGetConfigAttrs(
    SaImmAccessorHandleT accessorHandle, const SaNameT *objectName,
    SaImmAttrValuesT_2 ***attributes);

EXTERN_C SaAisErrorT
immutil_saImmOmAccessorFinalize(SaImmAccessorHandleT accessorHandle);

EXTERN_C SaAisErrorT immutil_saImmOmClassCreate_2(
    SaImmCcbHandleT immCcbHandle, const SaImmClassNameT className,
    const SaImmClassCategoryT classCategory,
    const SaImmAttrDefinitionT_2 **attrDefinitions);

EXTERN_C SaAisErrorT immutil_saImmOmClassDelete(
    SaImmCcbHandleT immCcbHandle, const SaImmClassNameT className);

/**
 * Wrapper for saImmOmSearchInitialize_2/_o2/_o3 The SA_AIS_ERR_NOT_EXIST error
 * code is not considered an error and is always returned even if
 * "errorsAreFatal" is set.
 */
EXTERN_C SaAisErrorT immutil_saImmOmSearchInitialize_2(
    SaImmHandleT immHandle, const SaNameT *rootName, SaImmScopeT scope,
    SaImmSearchOptionsT searchOptions,
    const SaImmSearchParametersT_2 *searchParam,
    const SaImmAttrNameT *attributeNames, SaImmSearchHandleT *searchHandle);

EXTERN_C SaAisErrorT immutil_saImmOmSearchInitialize_o2(
    SaImmHandleT immHandle, const char *rootName, SaImmScopeT scope,
    SaImmSearchOptionsT searchOptions,
    const SaImmSearchParametersT_2 *searchParam,
    const SaImmAttrNameT *attributeNames, SaImmSearchHandleT *searchHandle);

EXTERN_C SaAisErrorT immutil_saImmOmSearchInitialize_o3(
    SaImmHandleT immHandle, const char *rootName, SaImmScopeT scope,
    SaImmSearchOptionsT searchOptions,
    const SaImmSearchParametersT_2 *searchParam,
    const SaImmAttrNameT *attributeNames, SaImmSearchHandleT *searchHandle);

/**
 * Wrapper for saImmOmSearchFinalize
 */
EXTERN_C SaAisErrorT
immutil_saImmOmSearchFinalize(SaImmSearchHandleT searchHandle);

/**
 * Wrapper for saImmOmSearchNext_2/o2/_o3 The SA_AIS_ERR_NOT_EXIST error code is
 * not considered an error and is always returned even if "errorsAreFatal" is
 * set.
 */
EXTERN_C SaAisErrorT immutil_saImmOmSearchNext_2(
    SaImmSearchHandleT searchHandle, SaNameT *objectName,
    SaImmAttrValuesT_2 ***attributes);

EXTERN_C SaAisErrorT
immutil_saImmOmSearchNext_o2(SaImmSearchHandleT searchHandle, char **objectName,
                             SaImmAttrValuesT_2 ***attributes);

EXTERN_C SaAisErrorT
immutil_saImmOmSearchNext_o3(SaImmSearchHandleT searchHandle, char **objectName,
                             SaImmAttrValuesT_2 ***attributes);

/**
 * Wrapper for saImmOmAdminOwnerClear/_o2/_o3
 */
EXTERN_C SaAisErrorT immutil_saImmOmAdminOwnerClear(SaImmHandleT immHandle,
                                                    const SaNameT **objectNames,
                                                    SaImmScopeT scope);

EXTERN_C SaAisErrorT immutil_saImmOmAdminOwnerClear_o2(SaImmHandleT immHandle,
                                                       const char **objectNames,
                                                       SaImmScopeT scope);

EXTERN_C SaAisErrorT immutil_saImmOmAdminOwnerClear_o3(SaImmHandleT immHandle,
                                                       const char **objectNames,
                                                       SaImmScopeT scope);

/**
 * Wrapper for saImmOiFinalize
 */
EXTERN_C SaAisErrorT immutil_saImmOiFinalize(SaImmOiHandleT immOiHandle);

/**
 * Wrapper for saImmOmAdminOwnerInitialize
 */
EXTERN_C SaAisErrorT immutil_saImmOmAdminOwnerInitialize(
    SaImmHandleT immHandle, const SaImmAdminOwnerNameT admOwnerName,
    SaBoolT relOwnOnFinalize, SaImmAdminOwnerHandleT *ownerHandle);

/**
 * Wrapper for saImmOmAdminOwnerFinalize
 */
EXTERN_C SaAisErrorT
immutil_saImmOmAdminOwnerFinalize(SaImmAdminOwnerHandleT ownerHandle);

/**
 * Wrapper for saImmOmCcbInitialize
 */
EXTERN_C SaAisErrorT immutil_saImmOmCcbInitialize(
    SaImmAdminOwnerHandleT ownerHandle, SaImmCcbFlagsT ccbFlags,
    SaImmCcbHandleT *immCcbHandle);

/**
 * Wrapper for saImmOmCcbFinalize
 */
EXTERN_C SaAisErrorT immutil_saImmOmCcbFinalize(SaImmCcbHandleT immCcbHandle);

/**
 * Wrapper for saImmOmCcbApply
 */
EXTERN_C SaAisErrorT immutil_saImmOmCcbApply(SaImmCcbHandleT immCcbHandle);

/**
 * Wrapper for saImmOmCcbAbort
 */
EXTERN_C SaAisErrorT immutil_saImmOmCcbAbort(SaImmCcbHandleT immCcbHandle);

/**
 * Wrapper for saImmOmCcbValidate
 */
EXTERN_C SaAisErrorT immutil_saImmOmCcbValidate(SaImmCcbHandleT immCcbHandle);

/**
 * Wrapper for saImmOmAdminOwnerSet/_o2/_o3
 */
EXTERN_C SaAisErrorT
immutil_saImmOmAdminOwnerSet(SaImmAdminOwnerHandleT ownerHandle,
                             const SaNameT **name, SaImmScopeT scope);

EXTERN_C SaAisErrorT immutil_saImmOmAdminOwnerSet_o2(
    SaImmAdminOwnerHandleT ownerHandle, const char **name, SaImmScopeT scope);

EXTERN_C SaAisErrorT
immutil_saImmOmAdminOwnerSet_o3(SaImmAdminOwnerHandleT adminOwnerHandle,
                                const char **objectNames, SaImmScopeT scope);

/**
 * Wrapper for saImmOmAdminOwnerRelease/_o2/_o3
 */
EXTERN_C SaAisErrorT
immutil_saImmOmAdminOwnerRelease(SaImmAdminOwnerHandleT ownerHandle,
                                 const SaNameT **name, SaImmScopeT scope);

EXTERN_C SaAisErrorT immutil_saImmOmAdminOwnerRelease_o2(
    SaImmAdminOwnerHandleT ownerHandle, const char **name, SaImmScopeT scope);

EXTERN_C SaAisErrorT immutil_saImmOmAdminOwnerRelease_o3(
    SaImmAdminOwnerHandleT adminOwnerHandle, const char **objectNames,
    SaImmScopeT scope);

/**
 * Wrapper for saImmOmAdminOperationInvoke_o2
 */
EXTERN_C SaAisErrorT immutil_saImmOmAdminOperationInvoke_o2(
    SaImmAdminOwnerHandleT ownerHandle, const SaNameT *objectName,
    SaImmContinuationIdT continuationId, SaImmAdminOperationIdT operationId,
    const SaImmAdminOperationParamsT_2 **params,
    SaAisErrorT *operationReturnValue, SaTimeT timeout,
    SaImmAdminOperationParamsT_2 ***returnParams);

/**
 * Wrapper for saImmOmAdminOperationInvoke_o214
 */
EXTERN_C SaAisErrorT immutil_saImmOmAdminOperationInvoke_o214(
    SaImmAdminOwnerHandleT ownerHandle, const char *objectName,
    SaImmContinuationIdT continuationId, SaImmAdminOperationIdT operationId,
    const SaImmAdminOperationParamsT_2 **params,
    SaAisErrorT *operationReturnValue, SaTimeT timeout,
    SaImmAdminOperationParamsT_2 ***returnParams);

/**
 * Wrapper for saImmOmAdminOperationInvoke_2/_o3
 */
EXTERN_C SaAisErrorT immutil_saImmOmAdminOperationInvoke_2(
    SaImmAdminOwnerHandleT ownerHandle, const SaNameT *objectName,
    SaImmContinuationIdT continuationId, SaImmAdminOperationIdT operationId,
    const SaImmAdminOperationParamsT_2 **params,
    SaAisErrorT *operationReturnValue, SaTimeT timeout);

EXTERN_C SaAisErrorT immutil_saImmOmAdminOperationInvoke_o3(
    SaImmAdminOwnerHandleT ownerHandle, const char *objectName,
    SaImmContinuationIdT continuationId, SaImmAdminOperationIdT operationId,
    const SaImmAdminOperationParamsT_2 **params,
    SaAisErrorT *operationReturnValue, SaTimeT timeout,
    SaImmAdminOperationParamsT_2 ***returnParams);

/**
 * Wrapper for saImmOmAdminOperationInvokeAsync_2/_o3
 */
EXTERN_C SaAisErrorT immutil_saImmOmAdminOperationInvokeAsync_2(
    SaImmAdminOwnerHandleT ownerHandle, SaInvocationT userInvocation,
    const SaNameT *objectName, SaImmContinuationIdT continuationId,
    SaImmAdminOperationIdT operationId,
    const SaImmAdminOperationParamsT_2 **params);

EXTERN_C SaAisErrorT immutil_saImmOmAdminOperationInvokeAsync_o3(
    SaImmAdminOwnerHandleT ownerHandle, SaInvocationT userInvocation,
    const char *objectName, SaImmContinuationIdT continuationId,
    SaImmAdminOperationIdT operationId,
    const SaImmAdminOperationParamsT_2 **params);

/**
 * Wrapper for saImmOmCcbObjectCreate_2/_o2/_o3
 */
EXTERN_C SaAisErrorT immutil_saImmOmCcbObjectCreate_2(
    SaImmCcbHandleT immCcbHandle, const SaImmClassNameT className,
    const SaNameT *parent, const SaImmAttrValuesT_2 **attrValues);

EXTERN_C SaAisErrorT immutil_saImmOmCcbObjectCreate_o2(
    SaImmCcbHandleT immCcbHandle, const SaImmClassNameT className,
    const char *parent, const SaImmAttrValuesT_2 **attrValues);

EXTERN_C SaAisErrorT immutil_saImmOmCcbObjectCreate_o3(
    SaImmCcbHandleT ccbHandle, const char *className,
    const char *const objectName, const SaImmAttrValuesT_2 **attrValues);

/**
 * Wrapper for saImmOmCcbObjectModify_2/_o2/_o3
 */
EXTERN_C SaAisErrorT immutil_saImmOmCcbObjectModify_2(
    SaImmCcbHandleT immCcbHandle, const SaNameT *objectName,
    const SaImmAttrModificationT_2 **attrMods);

EXTERN_C SaAisErrorT immutil_saImmOmCcbObjectModify_o2(
    SaImmCcbHandleT immCcbHandle, const char *objectName,
    const SaImmAttrModificationT_2 **attrMods);

EXTERN_C SaAisErrorT immutil_saImmOmCcbObjectModify_o3(
    SaImmCcbHandleT ccbHandle, const char *objectName,
    const SaImmAttrModificationT_2 **attrMods);

/**
 * Wrapper for saImmOmCcbObjectDelete/_o2/_o3
 */
EXTERN_C SaAisErrorT immutil_saImmOmCcbObjectDelete(
    SaImmCcbHandleT immCcbHandle, const SaNameT *objectName);

EXTERN_C SaAisErrorT immutil_saImmOmCcbObjectDelete_o2(
    SaImmCcbHandleT immCcbHandle, const char *objectName);

EXTERN_C SaAisErrorT immutil_saImmOmCcbObjectDelete_o3(
    SaImmCcbHandleT ccbHandle, const char *objectName);

/**
 * Wrapper for saImmOmCcbObjectRead
 */
EXTERN_C SaAisErrorT immutil_saImmOmCcbObjectRead(
    SaImmCcbHandleT ccbHandle, SaConstStringT objectName,
    const SaImmAttrNameT *attributeNames, SaImmAttrValuesT_2 ***attributes);

/**
 * Wrapper for saImmOmClassDescriptionGet_2
 */
EXTERN_C SaAisErrorT immutil_saImmOmClassDescriptionGet_2(
    SaImmHandleT immHandle, const SaImmClassNameT className,
    SaImmClassCategoryT *classCategory,
    SaImmAttrDefinitionT_2 ***attrDefinitions);

/**
 * Wrapper for saImmOmClassDescriptionMemoryFree_2
 */
EXTERN_C SaAisErrorT immutil_saImmOmClassDescriptionMemoryFree_2(
    SaImmHandleT immHandle, SaImmAttrDefinitionT_2 **attrDef);

/*@}*/

#endif
