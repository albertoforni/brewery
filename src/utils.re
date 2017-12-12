type buffer = string;
[@bs.send] external stringOfBuffer': buffer => string = "toString";
let makeBuffer: string => buffer = (a) => a;
let stringOfBuffer: string => string = (buffer) => makeBuffer(buffer) |> stringOfBuffer';


[@bs.val][@bs.scope "JSON"] external jsonStringify': 'any => Js.Nullable.t(string) => int => string = "stringify";
let jsonStringfy = (a) =>
  try (Some(jsonStringify'(a, Js.Nullable.null, 2))) {
  | _ => None
  }
