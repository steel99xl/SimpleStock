# SimpleStock
Gives simple stock informatoin 
such as price, short interests, and avalible volume averages

The Python version can give you the most information


The C++ version is faster but and manage PortfoloiFiles but only give basic ticker information

# Requirements
## General
internet connection


## C++
libcurl


## Python
### out of date
python3


# Usage
```bash
# C++ version
./Stock [TICKER]
./Stock -i
./Stock -l [PortfolioFile]
./Stock -n [NewPortfolioFile] [StartingCash]

# Python version
./Stock.py [TICKER]
./Stock.py -s       get most shorted stocks
./Stock.py -f [PortfolioFile] [threadcount](optinal) loads and displays portfolio
```
