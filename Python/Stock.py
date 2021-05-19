#!/usr/bin/python3
## by steel99xl
## TODO: add cash system to "buy" and "sell" stocks in portfolio(s) | reload cache every 5 min
## Imports
import requests
import pathlib
import sys
import numpy as np
from threading import Thread


## Define urls and vars
Highshort = "https://www.highshortinterest.com/"
Iborrowdesk = "https://iborrowdesk.com/api/ticker/"
shortP1 = "https://finance.yahoo.com/quote/"
shortP2 = "/key-statistics"
YahooP1 = "https://query1.finance.yahoo.com/v8/finance/chart/"
YahooP2 = "?region=US&lang=en-US&includePrePost=false&interval=1d&useYfid=true&range=1d&corsDomain=finance.yahoo.com&.tsrc=finance"
RESULTS = []
Threads = 1000
Jobs = []


## progress Bar
def progress(count, total, suffix=''):
    bar_len = 60
    filled_len = int(round(bar_len * count / float(total)))

    percents = round(100.0 * count / float(total), 1)
    bar = '$' * filled_len + '-' * (bar_len - filled_len)

    sys.stdout.write('[%s] %s%s ...%s\r' % (bar, percents, '%', suffix))
    sys.stdout.flush()

## Get url response
def url(url):
    WebPage = requests.get(url)
    Data = WebPage.text
    return Data


## Get list of largest shorts
def highshort():
    Info = []
    Data = url(Highshort)
    Data = Data.split('</tr>')
    #from 4 - 44
    i = 4
    while(i <= 44):
        if(i == 29):
            i += 1
            continue
        Buff = Data[i]
        Buff = Buff.split('>')
        Ticker = Buff[3].strip('</a')
        Per = Buff[10].strip('</td')
        Out = Ticker + " - " + Per
        Info.append(Out)
        i += 1

    x = 0
    while(x <= len(Info)-4):
        print(Info[x] +' | ' + Info[x+1] +' | ' + Info[x+2] +' | ' + Info[x+3])
        x += 4


## Get Average Avalible Volume
def volume(Ticker):
    Data = url(Iborrowdesk + Ticker.upper())
    Data = Data.split('[')
    AData = Data[1].split('{')
    BData = Data[2].split('{')
    i = 1
    Total = 0
    while(i < len(AData)):
        X = AData[i].split(':')
        X = X[1].split(',')
        Total += int(X[0])
        i += 1

    print("volume Avalible Volume for today = "+ str(Total/i))
    i = 1
    Total = 0
    while(i < len(BData)):
        X = BData[i].split(':')
        X = X[1].split(',')
        Total += int(X[0])
        i += 1

    print("volume Avalible Volume for the year = "+ str(Total/i))


## Get Open, Current, and Previous Close price
def Info(Ticker):
    RESULT = []
    path = 'cache/'+Ticker.upper()
    File = pathlib.Path(path)
    if(File.exists()):
        Data = cacheread(Ticker.upper())
    else:
        Data = cachemake(Ticker.upper())
        
    PData = Data.split(',')
    CPrice = PData[9].split(':')[1]
    YPrice = PData[10].split(':')[1]
    OData = Data.split('open')
    OData = OData[1].split('[')
    OData = OData[1].split(',')
    RESULT.append(float(YPrice))
    RESULT.append(float(OData[0].rstrip("]}")))
    RESULT.append(float(CPrice))

    RESULTS.append(RESULT)


## Get short information about specified Stock
def short(Ticker):
    Data = url(shortP1 + Ticker.upper() + shortP2)
    SData = Data.split("sharesPercentSharesOut")
    SData = SData[1].split("{")
    SData = SData[1].split('"')
    print("% Shares shorted = " +SData[5])


## Loads and displays portfolio files
def portfolioview(PFile):
    Names = []
    Amounts = []
    Prices = []
    LGTotal = []
    PValues = []

    Profile = open(PFile, 'r')
    Stocks = Profile.readlines()
    for Stock in Stocks:
        Buff = Stock.split(',')
        Names.append(Buff[0])
        Amounts.append(Buff[1])
        Prices.append(Buff[2].strip('\n'))

    Profile.close()

    for i in range(len(Names)):
        progress(i, len(Names), "Gathering Info...")
        Info(Names[i])

    for i in range(len(Names)):
        LGp = float(float(RESULTS[i][2])-float(Prices[i]))
        LGTotal.append(float(Amounts[i]) * LGp)
        LGp = 0.0
        PValues.append(float(Amounts[i]) * RESULTS[i][2])

    # yay arbitraty print statment
    print("\nTICKER | Amount | Payed | Current | Loss/Gain")

    for i in range(len(Names)):
        print(Names[i] + " | " + Amounts[i] + " | " + str(Prices[i]) +" | "+ str(RESULTS[i][2]) + " | " + str(LGTotal[i]))

    X = 0
    for i in LGTotal:
        X += i
    Y = 0
    for j in PValues:
        Y += j

    Z = 0
    for k in range(len(Amounts)):
        Z += float(Amounts[k]) * float(Prices[k])

    print("")
    print("-"*20)
    print("Initial Value = " + str(Z))
    print("Total Loss/Gain = " + str(X))
    print("Total Profile Value = " + str(Y))
    print("")


## Used by cachebulkmake for thread call
def cachearray(Stocks):
    for i in range(len(Stocks)):
        Stock = Stocks[i].strip('\n')
        Path = 'cache/'+Stock.upper()
        File = pathlib.Path(Path)
        if(File.exists() == False):
            Data = url(YahooP1 + Stock.upper() + YahooP2)
            with open (Path, 'w') as Tmp:
                Tmp.write(Data)
            Tmp.close()
        else:
            pass


## Threads the making of cache files from list file
def cachebulkmake(SFile, *UThreads):
    StockFile = open(SFile, 'r')
    Stocks = StockFile.readlines()
    StockFile.close()
    
    if(UThreads == "" ):
        UThreads = Threads
    
    StockGroups = np.array_split(Stocks, int(UThreads[0]))
    
    for i in range(len(StockGroups)):
        progress(i, len(StockGroups), "Gathering Info...")
        
        Group = Thread(target = cachearray(StockGroups[i]))
        
        Jobs.append(Group)
        
        Jobs[i].start()
        
        
## Makes Cache file if one is not present
def cachemake(Ticker):
    Path = 'cache/'+Ticker
    Data = url(YahooP1 + Ticker.upper() + YahooP2)
    with open (Path, 'w') as Tmp:
        Tmp.write(Data)
    Tmp.close()
    return Data


## Loads Cached files for parsing
def cacheread(Ticker):
    Path = 'cache/'+Ticker.upper()
    StockFile = open(Path, 'r')
    Data = StockFile.read()
    StockFile.close()
    return Data


## Basic Argument parsing
def main(argv):
    path = 'cache'
    File = pathlib.Path(path)
    if(File.exists() == False):
        File.mkdir(parents=True, exist_ok=True)

    if(len(argv) <= 0):
        print("Stock.py [TICKER]")
        print('[TICKER] -a             Get average volume of avalible shares')
        print('-s               Get list of the most shorted shares')
        print(' -f [FILE]       Import contents of file for portfolio view mode')
        print(' -c [FILE] [THEADS] Make cache files of stocks from list')
        return 1
    
    
    # lazy way of cheking for args

    if(argv[0] == '-s'):
        highshort()
    elif(argv[0] == '-f'):
        portfolioview(argv[1])
    elif(argv[0] == '-c'):
        if(len(argv) == 3):
            cachebulkmake(argv[1], argv[2])
        else:
            cachebulkmake(argv[1])
    else:
        Ticker = argv[0]

        try:
            Option = argv[1]
        except:
            Option = ""

        if(Option == "-a"):
            volume(Ticker)

        Info(Ticker)
        print("Previous Close = $" + str(RESULTS[0][0]))
        print("Open Price = $" + str(RESULTS[0][1]))
        print("Current Price = $" + str(RESULTS[0][2]))

        try:
            short(Ticker)
        except:
            print("No short info")

main(sys.argv[1:])
