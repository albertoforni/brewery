open Jest;
open Index;
open Expect;

let _ =

describe "brewery help" (fun () => {
  test "it returns help" (fun () => {
    let res = ref "";
    let system = {
      log: fun s => {
        res := s;
        ()
      },
      writeFile: fun _ _ => (),
    };

    Index.run system [|"node", "program", "help"|];

    expect !res |> toBe "here some help"
  });
  
  describe "brewery init" (fun () => {
    test "it creates a .brewery.json" (fun () => {
      let res = ref "";
      let writeFileRes = ref ("", "");
      let system = {
        log: fun s => {
          res := s;
          ()
        },
        writeFile: fun path content => writeFileRes := (path, content)
      };

      Index.run system [|"node", "program", "init"|];

      expect (
        !writeFileRes,
        !res
      ) |> toEqual (
        (Index.breweryConfig, "{cask: [], brew: []}"),
        ".brewery.json created"
      )
    });
  });
});
  