type result('a) =
  | Ok('a)
  | Error(string);

let bind = (result, fn) =>
  switch result {
  | Ok(s) => fn(s)
  | Error(s) => Error(s)
  };
let (>>=) = bind;

let map = (result, fn) =>
  switch result {
      | Ok(s) => Ok(fn(s))
      | Error(s) => Error(s)
  };

let (<$>) = map;
