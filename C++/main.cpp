#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
//#include <thread>
//#include <chrono>

// Only external library
#include <curl/curl.h>

// TODO : Either store in ram or load data from the file eatch time

// URL BANK
const char *Yahoo[2] = {"https://query1.finance.yahoo.com/v8/finance/chart/", "?region=US&lang=en-US&includePrePost=false&interval=1d&useYfid=true&range=1d&corsDomain=finance.yahoo.com&.tsrc=finance"};


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
std::string WebHandler(char *Ticker){
    CURL *curl;
    CURLcode res;
    std::string data;
    char url[256];
    strcpy(url,Yahoo[0]);
    strcat(url,Ticker);
    strcat(url,Yahoo[1]);

    curl = curl_easy_init();

    if(curl){
        // takes char * and const char * for url
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
        // Passed Provided string to the callback function noted above
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

        // This makes the curl connection
        res = curl_easy_perform(curl);
      }
    curl_easy_cleanup(curl);
    return data;
}

// Take String and returns array for 3 strings
// Only parses for price information
std::string *WebParser(std::string data){
    const char  *delchars = ",:op";
    int delcount[4] = {0,0,0,0};

    // oooo auto thanks clion
    auto *output = new std::string[3];
    // 3 buffers cause we need three diffrent bits of info and it makes it easy
    std::string buff1;
    std::string buff2;
    std::string buff3;

    for(int i = 0; i <= strlen(delchars)-1; i ++){
        int j = 0;
        int x = 0;
        while(j <= data.length()){
            if(data[j] == delchars[i]){
              x+= 1;
              switch(x){
                case(10):
                  if(i == 0){
                    delcount[0] = j;
                  }
                  break;

                case(11):
                  if(i == 0){
                    delcount[2] = j;
                  }
                  break;

                case(13):
                  if(i == 1){
                    delcount[1] = j;
                  }
                  break;

                case(14):
                  if(i == 1){
                    delcount[3] = j;
                  }
                  break;

                default:
                  break;
              }

              if(data[j] == delchars[2] && data[j+1] == delchars[3]){
                    buff3 = data.substr(j+7,6);
                }

              if(delcount[0] != 0 && delcount[1] != 0){
                    buff1 = data.substr(delcount[1]+1,delcount[0]-delcount[1]-1);
                    delcount[0] = 0;
                }
              if(delcount[2] != 0 && delcount[3] != 0){
                    buff2 = data.substr(delcount[3]+1,delcount[2]-delcount[3]-1);
                    delcount[2] = 0;
                }
            }
           j++;
        }

    }

    output[0] = buff1;
    output[1] = buff2;
    output[2] = buff3;

    return output;
}

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

void SavePortfolio(char *FilePath, std::vector<std::string> names,std::vector<std::string> amount, std::vector<std::string> price, std::vector<std::string> floatcash){
    std::ofstream File;

    File.open(FilePath, std::ios::out);
    if(File.is_open()){
        File << "FloatCash="+floatcash[0]+"\n";
        for(int i = 0; i < names.size(); i++){
            File << names[i] + "," + amount[i] + "," + price[i] + "\n";
        }
        File.close();
    }
    std::cout << "SAVE complete..." << std::endl;
}

void ParserPortfolio(std::vector<std::string> lines, std::vector<std::string> *names, std::vector<std::string> *amount, std::vector<std::string> *price, std::vector<std::string> *floatcash){
    std::string delims = ",=";
    int delcount[5] = {0,0,0,0,0};
    std::string data;

    names->clear();
    amount->clear();
    price->clear();
    floatcash->clear();

    // 3 buffers cause we need three diffrent bits of info and it makes it easy
    for(auto & line : lines) {
        delcount[0] = 0;
        delcount[1] = 0;
        for (int j = 0; j < line.length(); j++) {
            if (line[j] == delims[0]) {
                if (delcount[0] == 0) {
                    delcount[0] = j;
                } else {
                    delcount[1] = j;
                }
            }
            if (line[j] == delims[1]) {
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

void MarketPortfolio(std::vector<std::string> userinput, std::vector<std::string> *names, std::vector<std::string> *amount, std::vector<std::string> *price, std::vector<std::string> *floatcash ){
    std::vector<std::string> tmpnames = *names;
    std::vector<std::string> tmpprice = *price;
    std::vector<std::string> tmpamount = *amount;
    std::string *output;
    std::string data;
    char ticker[8];

    std::strcpy(ticker, userinput[1].c_str());
    data = WebHandler(ticker);
    output = WebParser(data);
    std::vector<std::string> cash = floatcash[0];
    floatcash->pop_back();

    if(std::strcmp(userinput[0].c_str(),"BUY") == 0) {

        if (std::stof(cash[0]) < std::stof(output[0]) * std::stof(userinput[2])) {
            std::cout << "ERROR low funds.." << std::endl;
            return;
        }
        floatcash->push_back(std::to_string(std::stof(cash[0]) - std::stof(output[0]) * std::stof(userinput[2])));

        for (int i = 0; i < names->size(); i++) {
            // Error happend down here some where...

            if (tmpnames[i] == userinput[1]) {

                tmpprice[i] = std::to_string(((std::stof(tmpprice[i]) * std::stof(tmpamount[i]) +
                                               std::stof(output[0]) * std::stof(userinput[2])) /
                                              (std::stof(tmpamount[i]) + std::stof(userinput[2]))));
                tmpamount[i] = std::to_string(std::stof(tmpamount[i]) + std::stof(userinput[2]));

                price->clear();
                amount->clear();

                for (int j = 0; j < tmpamount.size(); j++) {
                    price->push_back(tmpprice[j]);
                    amount->push_back(tmpamount[j]);
                }
                std::cout << "PURCHASE complete..." << std::endl;
                return;
            }
        }
        names->push_back(userinput[1]);
        price->push_back(output[0]);
        amount->push_back(userinput[2]);
        std::cout << "PURCHASE complete..." << std::endl;
        return;
    }

    if(std::strcmp(userinput[0].c_str(),"SELL") == 0){

        for (int i = 0; i < names->size(); i++) {
            // Error happend down here some where...

            if (tmpnames[i] == userinput[1]) {
                if((std::stof(tmpamount[i]) - std::stof(userinput[2])) < 0){
                    std::cout << "ERROR not enough shares..." << std::endl;
                    return;
                }

                tmpamount[i] = std::to_string(std::stof(tmpamount[i]) - std::stof(userinput[2]));

                price->clear();
                amount->clear();

                if(std::stof(tmpamount[i]) == 0){
                    tmpamount.erase(tmpamount.begin()+i);
                    tmpprice.erase(tmpprice.begin()+i);
                    tmpnames.erase(tmpnames.begin()+i);

                    names->clear();
                    for(int j = 0; j < tmpnames.size(); j++){
                        names->push_back(tmpnames[j]);
                    }
                }

                for (int j = 0; j < tmpamount.size(); j++) {
                    price->push_back(tmpprice[j]);
                    amount->push_back(tmpamount[j]);
                }
                floatcash->push_back(std::to_string(std::stof(cash[0]) + std::stof(output[0]) * std::stof(userinput[2])));
                std::cout << "SALE complete..." << std::endl;
                return;
            }
        }

        std::cout << "ERROR you dont own any " << userinput[1] << std::endl;
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

    for(int i = 0; i < userinput.length(); i++){
        userinput[i] = toupper(userinput[i]);
    }

    return userinput;
}

void DisplayPortfolio(std::vector<std::string> names, std::vector<std::string> amount, std::vector<std::string> price, std::vector<std::string> floatcash){
    // Calculates informaion based on loaded file
    float LossGain = 0.0;
    float TotalValue = 0.0;
    char ticker[8];
    std::string data;

    std::string *output;
    // 3 buffers cause we need three diffrent bits of info and it makes it easy
    std::cout << "TICKER | Amount | Buy Price | Current Price | Loss/Gain \n" << std::endl;
    for(int i = 0; i < names.size(); i++){

            std::strcpy(ticker,names[i].c_str());
            data = WebHandler(ticker);

            output = WebParser(data);

            LossGain += std::stof(amount[i]) * std::stof(output[0]) - std::stof(amount[i]) * std::stof(price[i]);
            TotalValue += std::stof(amount[i]) * std::stof(output[0]);

            std::cout << names[i] << " | " << amount[i] << " | " << price[i] << " | " << output[0] << " | " << std::stof(output[0]) - std::stof(price[i]) << std::endl;
    }
    std::cout << "\n\r------------------------------------------------" << std::endl;
    std::cout << "Float Cash : $" << std::stof(floatcash[0]) << std::endl;
    std::cout << "Initial Value : $" << TotalValue - LossGain + std::stof(floatcash[0]) << std::endl;
    std::cout << "Loss/Gain : $" << LossGain << std::endl;
    std::cout << "Total value : $" << TotalValue + std::stof(floatcash[0]) << std::endl;
    std::cout << "\n" << std::endl;


}

int main(int argc, char *argv[]){
    std::string data;
    std::string *output;

	if(argc < 2){
		std::cout << "ERROR use : " << std::endl;
		std::cout << "./Stock [TICKER] " << std::endl;
        std::cout << "./Stock -f [PortfolioFile] " << std::endl;

		return 1;
	}

    // Capitalizes user input
    int max = std::strlen(argv[1]);
    char input[max];
    int i = 0;
    while (argv[1][i]){
        input[i] = toupper(argv[1][i]);
        i++;
    }

    if(std::strcmp(argv[1], "-f") == 0 && argc == 3){
        // This could be a seperate function
        bool View = true;
        std::vector<std::string> lines;
        std::vector<std::string> names;
        std::vector<std::string> amount;
        std::vector<std::string> price;
        std::vector<std::string> floatcash;
        std::vector<std::string> userinput;

        // Load Profile
        lines = LoadPortfolio(argv[2]);
        ParserPortfolio(lines,&names,&amount,&price,&floatcash);
        DisplayPortfolio(names,amount,price,floatcash);

        while(View) {

            std::cout << "RELOAD | DISPLAY | BUY | SELL | SAVE | EXIT" << std::endl;
            userinput.push_back(UserAutoCap());

            if(std::strcmp(userinput[0].c_str(),"EXIT") == 0){
                std::cout << "EXITING app..." << std::endl;
                View = false;
            }
            if(std::strcmp(userinput[0].c_str(),"DISPLAY") == 0){
                DisplayPortfolio(names,amount,price,floatcash);
            }
            if(std::strcmp(userinput[0].c_str(),"RELOAD") == 0){
                lines = LoadPortfolio(argv[2]);
                ParserPortfolio(lines,&names,&amount,&price,&floatcash);
                DisplayPortfolio(names,amount,price,floatcash);
            }
            if(std::strcmp(userinput[0].c_str(),"BUY") == 0 || std::strcmp(userinput[0].c_str(),"SELL") == 0){
                std::cout << userinput[0] <<" MENU" << std::endl;
                std::cout << "Enter Ticker Name" << std::endl;
                userinput.push_back(UserAutoCap());
                std::cout << "Enter Amount" << std::endl;
                userinput.push_back(UserInput());

                MarketPortfolio(userinput,&names,&amount,&price,&floatcash);

                userinput.pop_back();
                userinput.pop_back();

            }
            if(std::strcmp(userinput[0].c_str(),"SAVE") == 0){
                std::cout << "SAVEING to file..." << std::endl;
                SavePortfolio(argv[2],names,amount,price,floatcash);
            }

            userinput.pop_back();
        }
        return 0;
    }

    data = WebHandler(input);

   // for(int count = 0;count < 10; ++count){
   //     ProgressBar(count, 10, '#');
   //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   // }

    output = WebParser(data);

    std::cout << "Curent price $";
    std::cout << std::stof(output[0]) << std::endl;
    std::cout << "Previous Close $";
    std::cout << std::stof(output[1]) << std::endl;
    std::cout << "Open price $";
    std::cout << std::stof(output[2]) << std::endl;

  	return 0;
}
