#ifndef __CORE_URL_SERVICE_HPP__
#define __CORE_URL_SERVICE_HPP__

#include <string>
#include <vector>

namespace service {

    namespace url_service {

        // Input assetUtil := <domain> ( <::> <domain> )* . <assetName>
        // sample: rabbit_house::chino.cappucchino
        // domain -> rabbit_house, chino
        // asset  -> cappuccino
        std::pair<
            std::vector<std::string>,
            std::string
        > getAssetNameFromUrl(std::string assetUrl);

    };
};

#endif