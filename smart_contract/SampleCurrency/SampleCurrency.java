
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

import java.util.HashMap;
import repository.DomainRepository;
import repository.AssetRepository;

// no package declaration
public class SampleCurrency {
    
    public static void put( String param ) {
        System.out.println("Hello in JAVA! in add");
        System.out.println("vvvvvvvv　param vvvvvvvv");
        System.out.println( param );
    }

    public static void put( String param ) {
        System.out.println("Hello in JAVA! in add");
        System.out.println("vvvvvvvv　param vvvvvvvv");
        System.out.println( param );
    }

    public static void remit(HashMap<String,String> params){
        System.out.println("Hello in JAVA! in contract");
        System.out.println("vvvvvvvv　params vvvvvvvv");
        for( String key : params.keySet() ) {
            System.out.println( key + " : " + params.get( key ) );
        }
    }

    public static void main(String[] argv) {
        // This is DB access test.

        System.out.println("Hello in JAVA! check repositories");

        System.out.println("DomainRepository");
        DomainRepository domain = new DomainRepository();
        domain.add("domain", 12345);
        // find, remove, ...

        System.out.println("AssetRepository");
        AssetRepository asset = new AssetRepository();
//        asset.add("MPTt3ULszCLGQqAqRgHj2gQHVnxn/DuNlRXR/iLMAn4=", "MyAsset", "some value");
        // find, remove, ...
        
        /*
        System.out.println("AccountRepository");
        AccountRepository account = new AccountRepository();
        account.add("account", 12345);
        // find, remove, ...
        */
    }

}
