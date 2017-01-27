javac -encoding UTF-8 SampleCurrency.java

# Domain Repository
javac -encoding UTF-8 repository/DomainRepository.java
javah -jni repository.DomainRepository
g++ -fPIC -shared -std=c++1y domain_repository_jni.cpp ../../core/util/datetime.cpp -I /usr/lib/jvm/java-1.8.0-openjdk-amd64/include -I  /usr/lib/jvm/java-1.8.0-openjdk-amd64/include/linux/ -o libDomainRepository.so

# Asset Repository
javac -encoding UTF-8 repository/AssetRepository.java
javah -jni repository.AssetRepository
g++ -fPIC -shared -std=c++1y asset_repository_jni.cpp ../../core/repository/domain/instance/asset_repository.cpp ../../core/infra/repository/world_state_repository_with_level_db.cpp -I /usr/lib/jvm/java-1.8.0-openjdk-amd64/include -I  /usr/lib/jvm/java-1.8.0-openjdk-amd64/include/linux/ -o libAssetRepository.so

# Account Repository
javac -encoding UTF-8 repository/AccountRepository.java
javah -jni repository.AccountRepository
g++ -fPIC -shared -std=c++1y account_repository_jni.cpp ../../core/repository/domain/instance/account_repository.cpp ../../core/infra/repository/world_state_repository_with_level_db.cpp -I /usr/lib/jvm/java-1.8.0-openjdk-amd64/include -I  /usr/lib/jvm/java-1.8.0-openjdk-amd64/include/linux/ -o libAccountRepository.so

# Java MakeMap
javac -encoding UTF-8 utility/JavaMakeMap.java
javah -jni utility.JavaMakeMap
g++ -fPIC -shared -std=c++1y java_make_map_jni.cpp -I /usr/lib/jvm/java-1.8.0-openjdk-amd64/include -I  /usr/lib/jvm/java-1.8.0-openjdk-amd64/include/linux/ -o libJavaMakeMap.so

java -Djava.library.path=. SampleCurrency
