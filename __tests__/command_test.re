open Jest;

open Index;

open Expect;

let defaultSystem = {log: (_) => (), writeFile: (_, _) => (), exec: (_) => "", fileExists: (_) => false };

let _ =
  describe(
    "brewery help",
    () =>
      test(
        "it returns help",
        () => {
          let logs = ref("");
          let system = {
            ...defaultSystem,
            log: (s) => {
              logs := s;
              ()
            }
          };
          Index.run(system, [|"node", "program", "help"|]);
          expect(logs^) |> toBe("here some help")
        }
      )
  );

describe(
  "brewery init",
  () => {
    test(
      "installs brew when it's not there",
      () => {
        let installBrew = ref(false);
        let system = {
          ...defaultSystem,
          exec: (command) => switch command {
            | "brew" => assert(false) /* when it errors it means it is not installed */
            | s when s == Index.installBrewScript => { installBrew := true; "install script output" }
            | "brew leaves" => "first\nsecond" 
            | "brew cask list" => "3\n4"
            | _ => ""
          }
        };
        Index.run(system, [|"node", "program", "init"|]);
        expect(installBrew^)
        |> toEqual(true)
      }
    );

    test(
      "creates a .brewery.json with the returned brew packages",
      () => {
        let logs = ref("");
        let writeFileRes = ref(("", ""));
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          writeFile: (path, content) => writeFileRes := (path, content),
          exec: (command) => switch command {
            | "brew" => "brew already installed"
            | "brew leaves" => "first\nsecond"
            | "brew cask list" => "3\n4"
            | _ => ""
          }
        };
        Index.run(system, [|"node", "program", "init"|]);
        let brewConfigJson = switch (Js.Json.stringifyAny({"cask": [|"3", "4"|], "brew": [|"first", "second"|]})) {
          | Some(s) => s
          | None => ""
        };
        expect((writeFileRes^, logs^))
        |> toEqual(((Index.breweryConfig, brewConfigJson), ".brewery.json created"))
      }
    );

    test(
      "returns an error when brew leaves throws and does not write .brewconfig",
      () => {
        let logs = ref("");
        let writeFileRes = ref(("", ""));
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          writeFile: (path, content) => writeFileRes := (path, content),
          exec: (command) => switch command {
            | "brew" => "brew already installed"
            | "brew leaves" => assert(false)
            | "brew cask list" => "3\n4"
            | _ => ""
          }
        };
        Index.run(system, [|"node", "program", "init"|]);
        expect((writeFileRes^, logs^))
        |> toEqual((("", ""), "error getting installed formulas"))
      }
    );

    test(
      "returns an error when brew cask install throws and does not write .brewconfig",
      () => {
        let logs = ref("");
        let writeFileRes = ref(("", ""));
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          writeFile: (path, content) => writeFileRes := (path, content),
          exec: (command) => switch command {
            | "brew" => "brew already installed"
            | "brew leaves" => "3\n4"
            | "brew cask list" => assert(false)
            | _ => ""
          }
        };
        Index.run(system, [|"node", "program", "init"|]);
        expect((writeFileRes^, logs^))
        |> toEqual((("", ""), "error getting installed formulas"))
      }
    );

    test(
      "returns an error if .brewery.json is already there",
      () => {
        let logs = ref("");
        let writeFileRes = ref(("", ""));
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          writeFile: (path, content) => writeFileRes := (path, content),
          fileExists: (path) => if (path == Index.breweryConfig) true else assert(false)
        };
        Index.run(system, [|"node", "program", "init"|]);
        expect((writeFileRes^, logs^))
        |> toEqual((("", ""), Index.breweryConfig ++ " exists already"))
      }
    )
  }
);
