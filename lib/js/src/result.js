// Generated by BUCKLESCRIPT VERSION 2.1.1, PLEASE EDIT WITH CARE
'use strict';

var Block = require("bs-platform/lib/js/block.js");
var Curry = require("bs-platform/lib/js/curry.js");

function bind(result, fn) {
  if (result.tag) {
    return /* Error */Block.__(1, [result[0]]);
  } else {
    return Curry._1(fn, result[0]);
  }
}

function map(result, fn) {
  if (result.tag) {
    return /* Error */Block.__(1, [result[0]]);
  } else {
    return /* Ok */Block.__(0, [Curry._1(fn, result[0])]);
  }
}

var $great$great$eq = bind;

var $less$$great = map;

exports.bind            = bind;
exports.$great$great$eq = $great$great$eq;
exports.map             = map;
exports.$less$$great    = $less$$great;
/* No side effect */
