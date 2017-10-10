open Jest;
open Index;




let _ =

describe "brewery help" (fun () => {
  open Expect;

  test "it returns help" (fun () => {
    let res = ref "";
    module TestSystem: System = {
      let log s => {
        res := s;
        ()
      };
    };

    module Main' = Main TestSystem;

    Main'.run [|"node", "program", "help"|];

    expect !res |> toBe "here some help"
  })
});