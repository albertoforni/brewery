'use strict';

var List          = require("bs-platform/lib/js/list.js");
var $$Array       = require("bs-platform/lib/js/array.js");
var Block         = require("bs-platform/lib/js/block.js");
var Curry         = require("bs-platform/lib/js/curry.js");
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
  var command = param[0];
  switch (command) {
    case 0 : 
        return /* Ok */Block.__(0, ["here some help"]);
    case 1 : 
        console.log("I don't know " + (commandToString(command) + " command"));
        return /* Error */Block.__(1, [commandToString(command)]);
    case 2 : 
        Child_process.execSync("brew install " + List.hd(param[1]), { });
        return /* Ok */Block.__(0, [""]);
    
  }
}

function log(prim) {
  console.log(prim);
  return /* () */0;
}

var System = /* module */[/* log */log];

function Main(System) {
  var run = function (stdin) {
    var res = execCommand(getCommand(parseArguments(stdin)));
    return Curry._1(System[/* log */0], res[0]);
  };
  return /* module */[/* run */run];
}

function run(stdin) {
  var res = execCommand(getCommand(parseArguments(stdin)));
  console.log(res[0]);
  return /* () */0;
}

run(Process.argv);

exports.parseArguments  = parseArguments;
exports.commandOfString = commandOfString;
exports.commandToString = commandToString;
exports.getCommand      = getCommand;
exports.execCommand     = execCommand;
exports.System          = System;
exports.Main            = Main;
/*  Not a pure module */
