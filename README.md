# Brewery [![Build Status](https://travis-ci.org/albertodotcom/brewery.svg?branch=master)](https://travis-ci.org/albertodotcom/brewery)
A teeny tiny wrapper around the awesome [Homebrew](https://brew.sh/) that writes your **brew** and **cask** formulas in a `.breweryfile.json` so that you can save it in your `dotfiles`

## Usage
Install `brewery` globally via `npm` or `yarn`
```
npm install -g brewery
```
or
```
yarn global add brewery
```
then take all your brew and brew cask already installed formulas and create `.breweryfile.json` in your `HOME` folder 
```
brewery init
```

for help run `brewery`
```
Hi from brewery 🍻  here some help

help                          - shows this output
init                          - creates .breweryfile.json in your HOME folder with the formulas that are currently installed in brew and brew cask
install [cask] [formula]      - installs the formula and adds it to .breweryfile.json
list                          - shows the installed formulas
uninstall [cask] [formula]    - uninstalls a formula and removes it from .breweryfile.json
```
