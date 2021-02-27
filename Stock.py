#!/bin/python3

## by steel99xl

## Imports
import urllib
import requests
import sys

# load urls from config file
# TODO : ^

Base = "https://iborrowdesk.com/api/ticker/"
# This is to get info for market price
PUrl = "https://query1.finance.yahoo.com/v8/finance/chart/"
PUrl2 = "?region=US&lang=en-US&includePrePost=false&interval=1d&useYfid=true&range=1d&corsDomain=finance.yahoo.com&.tsrc=finance"


## Get Average Avalible Volume
def Avg(Ticker):
    URL = Base + Ticker.upper()
    WebPage = urllib.request.urlopen(URL)
    Data = str(WebPage.read())
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

    while(i < len(BData)):
        X = BData[i].split(':')
        X = X[1].split(',')
        Total += int(X[0])
        i += 1

    print("Avg Avalible Volume for the year = "+ str(Total/i))

## Get Open, Current, and Previous Close price
def Price(Ticker):
    URL = PUrl + Ticker.upper() + PUrl2
    WebPage = urllib.request.urlopen(URL)
    Data = str(WebPage.read())
    PData = Data.split(',')
    CPrice = PData[9].split(':')[1]
    YPrice = PData[10].split(':')[1]
    OData = Data.split('open')
    OData = OData[1].split('[')
    OData = OData[1].split(',')

    print("Previous Close = $" + YPrice)
    print("Current Price = $" + CPrice)
    print("Open Price = $" + OData[0])


## Basic Argument parsing 
def main(argv):
    if(len(argv) <= 0):
        print("Stock.py [TICKER] <options>")
        print(' -a             Get average volume of avalible shares')
        return 1
    Ticker = argv[0]

    # lazy way of cheking for args
    try:
        Option = argv[1]

    except:
        Option = ""

    if(Option == "-a"):
        Avg(Ticker)
    Price(Ticker)

main(sys.argv[1:])
