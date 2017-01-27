/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
     http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package repository;

import java.util.HashMap;

public class DomainRepository {

    static {
        System.loadLibrary("DomainRepository");
    }
    
    public static native void     add(String key, String value);
    public static native void     add(String key, int value);   // Notice: singed only
    public static native void     add(String key, long value);  // Notice: singed only
    public static native void     add(String key, double value);

    public static native HashMap<String,String> findOne(String key);
    public static native void     update(String key, String newValue);
    public static native void     remove(String key);
    //std::vector<std::unique_ptr<T> > findAll(std::string key);
    //std::unique_ptr<T> findOrElse(std::string key, T defaultVale);
    //bool isExist(std::string key);

}