open Jest;

open Index;

open Expect;

let _ =
  describe("brewery help", () => {
      test("it returns help", () => {
          let res = ref("");
          let system = {
            log: (s) => {
              res := s;
              ()
            },
            writeFile: (_, _) => ()
          };
          Index.run(system, [|"node", "program", "help"|]);
          expect(res^) |> toBe("here some help")
        }
      );
      describe("brewery init", () =>
          test("it creates a .brewery.json", () => {
              let res = ref("");
              let writeFileRes = ref(("", ""));
              let system = {
                log: (s) => {
                  res := s;
                  ()
                },
                writeFile: (path, content) => writeFileRes := (path, content)
              };
              Index.run(system, [|"node", "program", "init"|]);

              let Some(brewConfigJson) = Js.Json.stringifyAny({"cask": [||], "brew": [||]});

              expect((writeFileRes^, res^))
              |> toEqual(((Index.breweryConfig, brewConfigJson), ".brewery.json created"))
            }
          )
      )
    }
  );
