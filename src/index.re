let parseArguments (args: array string): list string => {
  Array.sub args 2 (Array.length args - 2)
  |> Array.to_list
};

type command =
  | Help
  | Unknown
  | Install
  ;
let commandOfString (command: string): command =>
  command
  |> String.lowercase
  |> fun
    | "help" => Help
    | "install" => Install
    | _ => Unknown
;

type result =
  | Ok string
  | Error string
  ;

let commandToString (command: command): string =>
  switch command {
    | Help => "help"
    | Install => "install"
    | Unknown => "unknown"
  };

let getCommand args => {
    switch args {
     | [command, ...options] => (commandOfString command, options)
     | _ => (Unknown, []);
    };
  };

let execCommand (command: command, options): result => {
  switch command {
    | Help => Ok "here some help"
    | Install => Node_child_process.execSync ("brew install " ^ (List.hd options)) (Node_child_process.option ()); Ok "";
    | Unknown => Js.log ("I don't know " ^ commandToString command ^ " command"); Error (commandToString command)
  };
};

module type System = {
  let log: string => unit;
};

module System: System = {
  let log = Js.log;
};

module Main = fun (System: System) => {
  let run stdin =>
    parseArguments stdin
    |> getCommand
    |> execCommand
    |> fun res => switch res {
      | Ok resText => System.log resText
      | Error resText => System.log resText
    };
};

let () = {
  module Main' = Main System;
  Main'.run Node_process.argv;
}
  /* |> fun
    | [command, ...options] => let _ = exec (commandOfString command);
    | _ => exec (Unknown);
    ; */
  /* |> fun [command, ...options] => let _ = exec (commandOfString command)  */
  /* Node_fs.readFileAsUtf8Sync "./tests/.brewery.json"
  |> Node_fs.writeFileAsUtf8Sync "./tests/.output.json" */
  /* ; */