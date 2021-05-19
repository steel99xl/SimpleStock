#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <fstream>

// Only external library
#include <curl/curl.h>

// TODO : Either store in ram or load data from the file eatch time

// URL BANK
const char *Yahoo[2] = {"https://query1.finance.yahoo.com/v8/finance/chart/", "?region=US&lang=en-US&includePrePost=false&interval=1d&useYfid=true&range=1d&corsDomain=finance.yahoo.com&.tsrc=finance"};


// Visual Progress Bar
// maybe skipp this
void ProgressBar(int count, int total, char mark){
    int barlen = 60;
    int filledlen = barlen * count / total;
    //double present = 100.0 * float(count) / float(total);
    //std::cout << present;
    //std::cout << filledlen << std::endl;
    for(int i = 0; i <= filledlen; i++) {
        printf("..%c",mark);
    }
    printf("\r");

}

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

// Takes Ticker and string to return data to
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
        // takes char * and const char *
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
    int max = data.length();

    // oooo auto thanks clion
    auto *output = new std::string[3];
    // 3 buffers cause we need three diffrent bits of info and it makes it easy
    std::string buff1;
    std::string buff2;
    std::string buff3;

    for(int i = 0; i <= strlen(delchars)-1; i ++){
        int j = 0;
        int x = 0;
        while(j <= max){
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

void PortfolioView(char *FilePath){
    // Loads files and puts each line in a vector
    std::vector<std::string> lines = {};
    std::fstream File;
    //std::cout << FilePath << std::endl;
    File.open(FilePath, std::ios::in);
    if(File.is_open()){
        std::string line;
        while (getline(File, line)) {
        lines.push_back(line);
        }
        File.close();
    }
    else{
        std::cout << "ERROR could not load file..." << std::endl;
        return;
    }

    // Calculates informaion based on loaded file
    float LossGain = 0.0;
    float TotalValue = 0.0;
    std::string delims = ",";
    int delcount[2] = {0,0};
    std::string data;
    char ticker[6];

    // oooo auto thanks clion
    std::string *output;
    // 3 buffers cause we need three diffrent bits of info and it makes it easy
    std::string name;
    std::string amount;
    std::string price;
    std::cout << "TICKER | Amount | Buy Price | Current Price | Loss/Gain \n" << std::endl;
    for(int i = 0; i < lines.size(); i++){
        delcount[0] = 0;
        delcount[1] = 1;
        for(int j = 0; j < lines[i].length(); j++){
            if(lines[i][j] == delims[0]){
                if(delcount[0] == 0){
                    delcount[0] = j;
                }
                else{
                    delcount[1] = j;
                }

            }
        }
        name = lines[i].substr(0,delcount[0]);
        amount = lines[i].substr(delcount[0]+1,delcount[1]-delcount[0]-1);
        price = lines[i].substr(delcount[1]+1,lines[i].length()-1);

        std::strcpy(ticker,name.c_str());
        data = WebHandler(ticker);

        output = WebParser(data);

        LossGain += std::stof(amount) * std::stof(output[0]) - std::stof(amount) * std::stof(price);
        TotalValue += std::stof(amount) * std::stof(output[0]);

        std::cout << name << " | " << amount << " | " << price << " | " << output[0] << " | " << std::stof(output[0]) - std::stof(price) << std::endl;
    }
    std::cout << "\n\r------------------------------------------------" << std::endl;
    std::cout << "Initial Value : $" << TotalValue - LossGain << std::endl;
    std::cout << "Loss/Gain : $" << LossGain << std::endl;
    std::cout << "Total value : $" << TotalValue << std::endl;

    std::cout << "\n" << std::endl;

}

int main(int argc, char *argv[]){
    std::string data;
    std::string *output;
    int i = 0;

	if(argc < 2){
		std::cout << "ERROR use : " << std::endl;
		std::cout << "./Stock [TICKER] " << std::endl;
        std::cout << "./Stock -f [PortfolioFile] " << std::endl;

		return 1;
	}

    // Capitalizes user input
    int max = std::strlen(argv[1]);
    char userinput[max];
    while (argv[1][i]){
        userinput[i] = toupper(argv[1][i]);
        i++;
    }

    if(std::strcmp(argv[1], "-f") == 0 && argc == 3){
        PortfolioView(argv[2]);
        return 0;
    }

    data = WebHandler(userinput);

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
