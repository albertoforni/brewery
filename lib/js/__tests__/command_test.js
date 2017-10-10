'use strict';

var Jest  = require("bs-jest/lib/js/src/jest.js");
var Curry = require("bs-platform/lib/js/curry.js");
var Index = require("../src/index.js");

describe("brewery help", (function () {
        return Jest.test("it returns help", (function () {
                      var res = [""];
                      var log = function (s) {
                        res[0] = s;
                        return /* () */0;
                      };
                      var TestSystem = /* module */[/* log */log];
                      var Main$prime = Index.Main(TestSystem);
                      Curry._1(Main$prime[/* run */0], /* array */[
                            "node",
                            "program",
                            "help"
                          ]);
                      return Jest.Expect[/* toBe */2]("here some help")(Jest.Expect[/* expect */0](res[0]));
                    }));
      }));

/*  Not a pure module */
