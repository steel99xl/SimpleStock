#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
//#include <thread>
//#include <chrono>

// Only external library
#include <curl/curl.h>

// TODO : N/A

// URL BANK
const char *Yahoo[2] = {"https://query1.finance.yahoo.com/v8/finance/chart/", "?region=US&lang=en-US&includePrePost=false&interval=1d&useYfid=true&range=1d&corsDomain=finance.yahoo.com&.tsrc=finance"};
const char * Crypto = "https://web-api.coinmarketcap.com/v1/cryptocurrency/quotes/latest?id=2781,3526,3537,2821,3527,2782,3528,3531,3530,3533,3532,2832,3529,2783,2814,3549,2784,2786,2787,2820,3534,2815,3535,2788,2789,3536,3538,2790,3539,3540,3541,3542,2792,2793,2818,2796,2794,3544,3543,2795,3545,2797,3546,3551,3547,3550,3548,3552,3556,2800,2816,2799,3555,3558,3554,3557,3559,3561,2811,2802,3560,2819,2801,3562,2804,3563,2822,2803,2805,2791,3564,2817,2806,3566,3565,2808,2812,2798,3567,3573,3553,2807,2785,2809,3569,3568,2810,3570,2824,2813,3571,3572,2823,1,1027,2010,1839,6636,52,1975,2,512,1831,7083,74,9023,9022&convert_id=2781";

// Based on example from libcurl
size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s){
    size_t newLength = size*nmemb;
    try{
        s->append((char*)contents, newLength);
    }
    catch(std::bad_alloc &e){
        // handle memory problem
        return 0;
    }
    return newLength;
}


// Takes Ticker and returns WebSite data as a String
std::string WebHandler(const char *Ticker, const char *mode) {
    CURL *curl;
    //CURLcode res;
    std::string data;

    char url[256];

    curl = curl_easy_init();

    if(curl){
        // takes char * and const char * for url
        if (std::strcmp(mode,"-c") == 0 || std::strcmp(mode,"C") == 0 ) {
            curl_easy_setopt(curl, CURLOPT_URL, Crypto);
        }

        if (std::strcmp(mode,"S") == 0) {
            strcpy(url, Yahoo[0]);
            strcat(url, Ticker);
            strcat(url, Yahoo[1]);
            curl_easy_setopt(curl, CURLOPT_URL, url);
        }

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
        // Passed Provided string to the callback function noted above
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

        // This makes the curl connection
        curl_easy_perform(curl);
      }
    curl_easy_cleanup(curl);
    return data;
}

// Only parses for price information
std::vector<std::string> WebParser(std::string data, const char *crypto = ""){
    const char  *delchars = ",:op";
    int delcount[4] = {0,0,0,0};

    std::vector<std::string> output;
    // 3 buffers cause we need three diffrent bits of info for stock information
    std::string buff1;
    std::string buff2;
    std::string buff3;

    if (strcmp(crypto,"") == 0) {
        for (int i = 0; i <= strlen(delchars) - 1; i++) {
            int j = 0;
            int x = 0;
            while (j <= data.length()) {
                if (data[j] == delchars[i]) {
                    x += 1;
                    switch (x) {
                        case (10):
                            if (i == 0) {
                                delcount[0] = j;
                            }
                            break;

                        case (11):
                            if (i == 0) {
                                delcount[2] = j;
                            }
                            break;

                        case (13):
                            if (i == 1) {
                                delcount[1] = j;
                            }
                            break;

                        case (14):
                            if (i == 1) {
                                delcount[3] = j;
                            }
                            break;

                        default:
                            break;
                    }

                    if (data[j] == delchars[2] && data[j + 1] == delchars[3]) {
                        buff3 = data.substr(j + 7, 6);
                    }

                    if (delcount[0] != 0 && delcount[1] != 0) {
                        buff1 = data.substr(delcount[1] + 1, delcount[0] - delcount[1] - 1);
                        delcount[0] = 0;
                    }
                    if (delcount[2] != 0 && delcount[3] != 0) {
                        buff2 = data.substr(delcount[3] + 1, delcount[2] - delcount[3] - 1);
                        delcount[2] = 0;
                    }
                }
                j++;
            }

        }

        output.push_back(buff1);
        output.push_back(buff2);
        output.push_back(buff3);

        return output;
    } else {
        std::size_t Name = data.find(crypto);
        std::size_t Price;
        std::size_t EPrice;
        Price = data.find("price", Name);
        EPrice = data.find(',', Price);
        buff1 = data.substr(Price + 7, EPrice - Price - 7);

        output.push_back(buff1);

        return output;
    }
}

// Loads portfolio from disk
std::vector<std::string> LoadPortfolio(char *FilePath) {
    // Loads files and puts each line in a vector
    std::vector<std::string> lines = {};
    std::fstream File;
    //std::cout << FilePath << std::endl;
    File.open(FilePath, std::ios::in);
    if (File.is_open()) {
        std::string line;
        while (getline(File, line)) {
            lines.push_back(line);
        }
        File.close();
    } else {
        std::cout << "ERROR could not load file..." << std::endl;
    }
    return lines;
}

// Writes changes to disk
void SavePortfolio(char *FilePath, std::vector<std::string> types,std::vector<std::string> names,std::vector<std::string> amount, std::vector<std::string> price, std::vector<std::string> floatcash){
    std::ofstream File;

    File.open(FilePath, std::ios::out);
    if(File.is_open()){
        File << "FloatCash="+floatcash[0]+"\n";
        for(int i = 0; i < names.size(); i++){
            if(strcmp(types[i].c_str(),"C") == 0){
               File << names[i] + ";" + amount[i] + ";" + price[i] + "\n";
            }
            else {
                File << names[i] + "," + amount[i] + "," + price[i] + "\n";
            }
        }
        File.close();
    }
    std::cout << "SAVE complete..." << std::endl;
}

void ParserPortfolio(std::vector<std::string> lines, std::vector<std::string> *types, std::vector<std::string> *names, std::vector<std::string> *amount, std::vector<std::string> *price, std::vector<std::string> *floatcash){
    std::string delims = ",;=";
    int delcount[5] = {0,0,0,0,0};

    types->clear();
    names->clear();
    amount->clear();
    price->clear();
    floatcash->clear();

    for(auto & line : lines) {
        delcount[0] = 0;
        delcount[1] = 0;
        for (int j = 0; j < line.length(); j++) {
            if (line[j] == delims[0]) {
                if (delcount[0] == 0) {
                    delcount[0] = j;
                } else {
                    delcount[1] = j;
                    types->push_back("S");
                }
            }

            if (line[j] == delims[1]) {
                if (delcount[0] == 0) {
                    delcount[0] = j;
                } else {
                    delcount[1] = j;
                    types->push_back("C");
                }
            }
            if (line[j] == delims[2]) {
                delcount[2] = j;
                // ya
                delcount[3] = delcount[4];
            }
        }

        if (delcount[0] != delcount[1]) {
            names->push_back(line.substr(0, delcount[0]));
            amount->push_back(line.substr(delcount[0] + 1, delcount[1] - delcount[0] - 1));
            price->push_back(line.substr(delcount[1] + 1, line.length() - 1));
        }
        delcount[4] += 1;
    }

    if(delcount[2] != 0){
        floatcash->push_back(lines[delcount[3]].substr(delcount[2] + 1,lines[delcount[3]].length() - 1));
    }
    else{
        floatcash->push_back("0.0");
    }

}

void MarketPortfolio(std::vector<std::string> userinput, std::vector<std::string> *types, std::vector<std::string> *names, std::vector<std::string> *amount, std::vector<std::string> *price, std::vector<std::string> *floatcash ){
    std::vector<std::string> tmptypes = *types;
    std::vector<std::string> tmpnames = *names;
    std::vector<std::string> tmpprice = *price;
    std::vector<std::string> tmpamount = *amount;
    std::vector<std::string> output;
    std::string data;
    char ticker[8];

    std::strcpy(ticker, userinput[2].c_str());

    data = WebHandler(ticker,userinput[1].c_str());

    if(strcmp(userinput[1].c_str(),"C") == 0){
        output = WebParser(data,ticker);
    }
    else{
        output = WebParser(data);
    }
    std::vector<std::string> cash = floatcash[0];

    if(std::strcmp(userinput[0].c_str(),"BUY") == 0) {

        if (std::stof(cash[0]) < std::stof(output[0]) * std::stof(userinput[3])) {
            std::cout << "ERROR low funds.." << std::endl;
            return;
        }

        floatcash->pop_back();
        floatcash->push_back(std::to_string(std::stof(cash[0]) - std::stof(output[0]) * std::stof(userinput[3])));

        for (int i = 0; i < names->size(); i++) {

            if (tmpnames[i] == userinput[2]) {

                tmpprice[i] = std::to_string(((std::stof(tmpprice[i]) * std::stof(tmpamount[i]) +
                                               std::stof(output[0]) * std::stof(userinput[3])) /
                                              (std::stof(tmpamount[i]) + std::stof(userinput[3]))));
                tmpamount[i] = std::to_string(std::stof(tmpamount[i]) + std::stof(userinput[3]));

                tmptypes[i] = userinput[1];
                price->clear();
                amount->clear();
                types->clear();

                for (int j = 0; j < tmpamount.size(); j++) {
                    price->push_back(tmpprice[j]);
                    amount->push_back(tmpamount[j]);
                    types->push_back(tmptypes[j]);
                }
                std::cout << "PURCHASE complete..." << std::endl;
                return;
            }
        }
        names->push_back(userinput[2]);
        price->push_back(output[0]);
        amount->push_back(userinput[3]);
        types->push_back(userinput[1]);
        std::cout << "PURCHASE complete..." << std::endl;
        return;
    }

    if(std::strcmp(userinput[0].c_str(),"SELL") == 0){

        for (int i = 0; i < names->size(); i++) {
            // Error happend down here some where...

            if (tmpnames[i] == userinput[2]) {
                if((std::stof(tmpamount[i]) - std::stof(userinput[3])) < 0){
                    std::cout << "ERROR not enough shares..." << std::endl;
                    return;
                }


                tmpamount[i] = std::to_string(std::stof(tmpamount[i]) - std::stof(userinput[3]));

                price->clear();
                amount->clear();


                if(std::stof(tmpamount[i]) == 0){
                    tmpamount.erase(tmpamount.begin()+i);
                    tmpprice.erase(tmpprice.begin()+i);
                    tmpnames.erase(tmpnames.begin()+i);
                    tmptypes.erase(tmptypes.begin()+i);

                    names->clear();
                    types->clear();
                    for(int j = 0; j < tmpnames.size(); j++){
                        names->push_back(tmpnames[j]);
                        types->push_back(tmptypes[j]);
                    }
                }

                for (int j = 0; j < tmpamount.size(); j++) {
                    price->push_back(tmpprice[j]);
                    amount->push_back(tmpamount[j]);
                }

                floatcash->pop_back();
                floatcash->push_back(std::to_string(std::stof(cash[0]) + std::stof(output[0]) * std::stof(userinput[3])));
                std::cout << "SALE complete..." << std::endl;
                return;
            }
        }

        std::cout << "ERROR you dont own any " << userinput[2] << std::endl;
        return;
    }


}

std::string UserInput(){
    std::string userinput;
    std::cout << "# ";
    std::cin >> userinput;
    return userinput;
}

std::string UserAutoCap(){
    std::string userinput;

    std::cout << "# ";
    std::cin >> userinput;

    for(char & i : userinput){
        i = toupper(i);
    }

    return userinput;
}

void DisplayPortfolio(std::vector<std::string> types,std::vector<std::string> names, std::vector<std::string> amount, std::vector<std::string> price, std::vector<std::string> floatcash){
    // Calculates informaion based on loaded file
    float LossGain = 0.0;
    float TotalValue = 0.0;
    char ticker[8];
    std::string data;
    std::vector<std::string> output;

    std::cout << "TICKER | Amount | Buy Price | Current Price | Loss/Gain \n" << std::endl;
    for(int i = 0; i < names.size(); i++){

            std::strcpy(ticker,names[i].c_str());
            data = WebHandler(ticker,types[i].c_str());

            if(strcmp(types[i].c_str(),"C") == 0){
                output = WebParser(data,names[i].c_str());

            }else{
                output = WebParser(data);
            }


            LossGain += std::stof(amount[i]) * std::stof(output[0]) - std::stof(amount[i]) * std::stof(price[i]);
            TotalValue += std::stof(amount[i]) * std::stof(output[0]);

            std::cout << names[i] << " | " << amount[i] << " | " << price[i] << " | " << output[0] << " | " << std::stof(amount[i]) * std::stof(output[0]) - std::stof(amount[i]) * std::stof(price[i]) << std::endl;
    }
    std::cout << "\n\r------------------------------------------------" << std::endl;
    std::cout << "Float Cash : $" << std::stof(floatcash[0]) << std::endl;
    std::cout << "Initial Value : $" << TotalValue - LossGain + std::stof(floatcash[0]) << std::endl;
    std::cout << "Loss/Gain : $" << LossGain << std::endl;
    std::cout << "Total value : $" << TotalValue + std::stof(floatcash[0]) << std::endl;
    std::cout << "\n" << std::endl;


}

// Main menu for Portfolio managment
void Portfolio(char *FilePath){
    bool View = true;
    std::vector<std::string> lines;
    std::vector<std::string> types;
    std::vector<std::string> names;
    std::vector<std::string> amount;
    std::vector<std::string> price;
    std::vector<std::string> floatcash;
    std::vector<std::string> userinput;
    std::string data;


    // Load Profile
    lines = LoadPortfolio(FilePath);
    ParserPortfolio(lines,&types,&names,&amount,&price,&floatcash);
    DisplayPortfolio(types, names,amount,price,floatcash);

    while(View) {

        std::cout << "RELOAD | VIEW | SEARCH | BUY | SELL | SAVE | EXIT" << std::endl;
        userinput.push_back(UserAutoCap());

        if(std::strcmp(userinput[0].c_str(),"EXIT") == 0){
            std::cout << "EXITING app..." << std::endl;
            View = false;
        }
        if(std::strcmp(userinput[0].c_str(),"VIEW") == 0){
            DisplayPortfolio(types,names,amount,price,floatcash);
        }
        if(std::strcmp(userinput[0].c_str(),"RELOAD") == 0){
            lines = LoadPortfolio(FilePath);
            ParserPortfolio(lines,&types,&names,&amount,&price,&floatcash);
            DisplayPortfolio(types,names,amount,price,floatcash);
        }
        if(std::strcmp(userinput[0].c_str(),"SEARCH") == 0){
            std::cout << "Stock or Crypto (S/C)" << std::endl;
            userinput.push_back(UserAutoCap());
            std::cout << "Enter Ticker" << std::endl;
            userinput.push_back(UserAutoCap());

            data = WebHandler(userinput[2].c_str(),userinput[1].c_str());
            if(strcmp(userinput[1].c_str(),"C") == 0){
                std::vector<std::string> output = WebParser(data,userinput[2].c_str());
                std::cout << "Current Price $";
                std::cout << output[0] << std::endl;
            }
            else{
                std::vector<std::string> output = WebParser(data);

                std::cout << "Curent price $";
                std::cout << output[0] << std::endl;
                std::cout << "Previous Close $";
                std::cout << output[1] << std::endl;
                std::cout << "Open price $";
                std::cout << output[2] << std::endl;
            }
            userinput.pop_back();
            userinput.pop_back();
        }
        if(std::strcmp(userinput[0].c_str(),"BUY") == 0 || std::strcmp(userinput[0].c_str(),"SELL") == 0){
            std::cout << userinput[0] <<" MENU" << std::endl;
            std::cout << "Stock or Crypto (S/C)" << std::endl;
            userinput.push_back(UserAutoCap());
            std::cout << "Enter Ticker" << std::endl;
            userinput.push_back(UserAutoCap());
            std::cout << "Enter Amount" << std::endl;
            userinput.push_back(UserInput());

            MarketPortfolio(userinput,&types,&names,&amount,&price,&floatcash);

            userinput.pop_back();
            userinput.pop_back();
            userinput.pop_back();

        }
        if(std::strcmp(userinput[0].c_str(),"SAVE") == 0){
            std::cout << "SAVEING to file..." << std::endl;
            SavePortfolio(FilePath,types,names,amount,price,floatcash);
        }

        userinput.pop_back();
    }
}

int main(int argc, char *argv[]){
    std::string data;
    std::vector<std::string> output;

	if(argc < 2){
		std::cout << "ERROR use : " << std::endl;
		std::cout << "./Stock [TICKER] " << std::endl;
		std::cout << "./Stock -c [CRYPTO] " << std::endl;
        std::cout << "./Stock -f [PortfolioFile] " << std::endl;

		return 1;
	}

	if(std::strcmp(argv[1], "-c") == 0 && argc == 3){
	    std::cout << "owo crypto" << std::endl;
	    data = WebHandler(argv[1], argv[1]);

	    // Parse Crypto Data
	    std::size_t Name = data.find(argv[2]);
	    std::size_t Price;
	    std::size_t EPrice;
	    Price = data.find("price",Name);
	    EPrice = data.find(',', Price);

	    // Give the current price
	    std::cout << "Current Price : $";
	    std::cout << data.substr(Price+7, EPrice - Price - 7) << std::endl;

	    return 0;
	}

    if(std::strcmp(argv[1], "-f") == 0 && argc == 3) {
        Portfolio(argv[2]);
        return 0;
    }

    // Capitalizes user input
    for(int i = 0; i< sizeof(argv[1]); i++){
        argv[1][i] = toupper(argv[1][i]);
        i++;
    }


    data = WebHandler(argv[1],"s");

    output = WebParser(data);

    std::cout << "Curent price $";
    std::cout << output[0] << std::endl;
    std::cout << "Previous Close $";
    std::cout << output[1] << std::endl;
    std::cout << "Open price $";
    std::cout << output[2] << std::endl;

  	return 0;
}
