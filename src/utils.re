let stringOfBuffer: string => string = [%bs.raw
  {|
    function stringOfBuffer (buffer) { return buffer.toString(); }
  |}
];

let jsonStringfy: 'any => option(string) =
  (dict) => {
    let jsonStringfy: 'any => string = [%bs.raw
      {|
        function(obj) {
          return JSON.stringify(obj, null, 2);
        }
      |}
    ];
    try (Some(jsonStringfy(dict))) {
    | _ => None
    }
  };