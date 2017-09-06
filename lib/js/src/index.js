'use strict';

var List          = require("bs-platform/lib/js/list.js");
var $$Array       = require("bs-platform/lib/js/array.js");
var $$String      = require("bs-platform/lib/js/string.js");
var Process       = require("process");
var Child_process = require("child_process");

function parseArguments(args) {
  return $$Array.to_list($$Array.sub(args, 2, args.length - 2 | 0));
}

function commandOfString(command) {
  var param = $$String.lowercase(command);
  switch (param) {
    case "help" : 
        return /* Help */0;
    case "install" : 
        return /* Install */2;
    default:
      return /* Unknown */1;
  }
}

function commandToString(command) {
  switch (command) {
    case 0 : 
        return "help";
    case 1 : 
        return "unknown";
    case 2 : 
        return "install";
    
  }
}

function getCommand(args) {
  if (args) {
    return /* tuple */[
            commandOfString(args[0]),
            args[1]
          ];
  } else {
    return /* tuple */[
            /* Unknown */1,
            /* [] */0
          ];
  }
}

function execCommand(param) {
  var options = param[1];
  var command = param[0];
  switch (command) {
    case 0 : 
        console.log("here some help");
        return /* Ok */0;
    case 1 : 
        console.log("I don't know " + (commandToString(command) + " command"));
        return /* Error */[commandToString(command)];
    case 2 : 
        Child_process.execSync("brew install " + List.hd(options), { });
        console.log(options);
        return /* Ok */0;
    
  }
}

console.log(execCommand(getCommand(parseArguments(Process.argv))));

exports.parseArguments  = parseArguments;
exports.commandOfString = commandOfString;
exports.commandToString = commandToString;
exports.getCommand      = getCommand;
exports.execCommand     = execCommand;
/*  Not a pure module */
