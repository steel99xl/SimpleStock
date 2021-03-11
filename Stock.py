#!/bin/python3
## by steel99xl
## Imports
import urllib
import requests
import sys


## Define urls
HighShort = "https://www.highshortinterest.com/"
Iborrowdesk = "https://iborrowdesk.com/api/ticker/"
YahooP1 = "https://query1.finance.yahoo.com/v8/finance/chart/"
YahooP2 = "?region=US&lang=en-US&includePrePost=false&interval=1d&useYfid=true&range=1d&corsDomain=finance.yahoo.com&.tsrc=finance"


## Get URL response
def Url(URL):
    WebPage = urllib.request.urlopen(URL)
    Data = str(WebPage.read())
    return Data


## Get list of largest shorts
def Short():
    Info = []
    Data = Url(HighShort)
    Data = Data.split('</tr>')
    
    
    # range of usefull informaton if from
    # 4 to 44 shiping 29 sence its a banner thing 
    i = 4
    while(i <= 44):
        #
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
def Avg(Ticker):
    Data = Url(Iborrowdesk + Ticker.upper())
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

    print("Avg Avalible Volume for today = "+ str(Total/i))
    i = 1
    Total = 0
    while(i < len(BData)):
        X = BData[i].split(':')
        X = X[1].split(',')
        Total += int(X[0])
        i += 1

    print("Avg Avalible Volume for the year = "+ str(Total/i))


## Get Open, Current, and Previous Close price
def Price(Ticker):
    Data = Url(YahooP1 + Ticker.upper() + YahooP2)
    PData = Data.split(',')
    CPrice = PData[9].split(':')[1]
    YPrice = PData[10].split(':')[1]
    OData = Data.split('open')
    OData = OData[1].split('[')
    OData = OData[1].split(',')
    print("Previous Close = $" + YPrice)
    print("Open Price = $" + OData[0])
    print("Current Price = $" + CPrice)


## Basic Argument parsing
def main(argv):
    if(len(argv) <= 0):
        print("Stock.py [TICKER] <options>")
        print(' -a             Get average volume of avalible shares')
        print(' -s               Get list of the most shorted shares')
        return 1
    if(argv[0] == "-s"):
        Short()

    Ticker = argv[0]
    
    
    # lazy way of cheking for args
    try:
        Option = argv[1]
    except:
        Option = ""
    if(Option == "-a"):
        Avg(Ticker)

    if(Ticker != '-s'):
        Price(Ticker)

main(sys.argv[1:])
