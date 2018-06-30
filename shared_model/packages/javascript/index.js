/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

const Module = require('./lib/irohalib.js')

/**
 * Get message of std::exception by Emscripten raw ptr.
 * Link: https://groups.google.com/d/msg/emscripten-discuss/ZRhTbpeF6SI/5bPSlpQyAwAJ
 * @param {number} ptr
 */
function whatEmscriptenException(ptr) {
  // 8 should be the vtable offset for the what function
  const fPtr = Module.HEAPU32[(Module.HEAPU32[(ptr) >> 2] + 8) >> 2]
  const message = Module.AsciiToString(Module.dynCall_ii(fPtr, ptr))

  return message
}

/**
 * Wrap class object into Proxy to catch errors from bulders' methods.
 * WARNING! We assume that wrapped object has just methods and constructors.
 * @param {*} obj Any non-primitive object
 * @returns {Proxy|*} 
 */
function wrap(obj) {
  if (obj === Object(obj)) {
    return new Proxy(obj, {
      // Wrap constructors to enable "get" traps on new objects
      construct(target, args) {
        return wrap(new target(...args))
      },

      // Wrap every object's method
      get(target, propKey, receiver) {
        const calledProperty = Reflect.get(...arguments)

        if (typeof propKey !== 'symbol' // Don't wrap built-in symbols!
          && typeof calledProperty === 'function') {
          // Returned function is an Emscripten exceptions catcher
          return (...args) => {
            try {
              return wrap(calledProperty.apply(target, args))
            } catch (e) {
              throw typeof e === 'number' ?
                new Error(whatEmscriptenException(e)) : e
            }
          }
        } else {
          return calledProperty
        }
      }
    })
  } else {
    return obj
  }
}

// Export proxyfied embinded classes
module.exports = Object.entries(Module)
  .filter(([propKey, prop]) => prop && prop.hasOwnProperty('argCount'))
  .map(([propKey, prop]) => [propKey, wrap(prop)])
  .reduce((obj, [k, v]) => Object.assign(obj, { [k]: v }), {})

// Export embinded enums
module.exports.Role = Module.Role
module.exports.Grantable = Module.Grantable
