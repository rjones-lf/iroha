#!/usr/bin/env groovy

def doDoxygen() {
    
    sh """
        doxygen Doxyfile
        #rsync docs/doxygen
    """
}

return this
