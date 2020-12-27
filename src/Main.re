open Reducer;
open State;
open Util;

[@react.component]
let make = () => {
  let (state, dispatch) = React.useReducer(reducer, default);

  let fetchCode = s => {
    let _ =
      Lwt.bind(
        HttpClient.get("/" ++ s),
        s => {
          dispatch @@ `Set_code(s);
          Lwt.return();
        },
      );
    ();
  };

  let fetchData = s => {
    let _ =
      Lwt.bind(
        HttpClient.get("/" ++ s),
        s => {
          log("Parse data");
          let data = Parse.parse_string(s);
          dispatch @@ `Set_pdata(data);
          log("Parse data done");
          log("Search main");
          let (xfile, xfilepath) =
            Parse.search_main_file(Parse.get_files(data));
          if (String.equal(xfile, "")) {
            log("Found no main file");
          } else {
            log("Found main file: " ++ xfile);
          };
          dispatch @@
          `Inspect_file(Inspect.File.Direct_location(xfile, xfilepath));
          Lwt.return();
        },
      );
    ();
  };

  React.useEffect0(() => {
    log("Initial data and code fetch");
    fetchData("analysis.xml");
    None;
  });

  React.useEffect1(
    () => {
      if (!String.equal(state.file_name, "")) {
        fetchCode(state.file_name);
        log("Fetched code: " ++ state.file_name);
      };
      None;
    },
    [|state.file_name|],
  );

  React.useEffect0(() => {
    log("Fetching CIL dump");
    let _ =
      Lwt.bind(
        HttpClient.get("/cilfile.dump"),
        s => {
          log("Fetched CIL dump");
          Js_of_ocaml.Js.Unsafe.global##.dump := s;
          let cil =
            try(Marshal.from_string(s, 0)) {
            | ex =>
              log(Printexc.to_string(ex));
              failwith("Cannot deserialize");
            };
          Cil.dumpFile(Cil.defaultCilPrinter, stdout, "main.c", cil);
          log("Printed " ++ cil.Cil.fileName);
          dispatch @@ `Set_cil(cil);
          Lwt.return();
        },
      );
    None;
  });

  React.useEffect1(
    () => {
      switch (state.inspect) {
      | Some(Func(f)) =>
        if (Option.is_none(Inspect.Func.dot_opt(f))) {
          let pattern = Js_of_ocaml.Regexp.regexp("/");
          let url =
            "dot/"
            ++ Js_of_ocaml.Regexp.global_replace(
                 pattern,
                 Inspect.Func.file_path(f),
                 "%252F",
               )
            ++ "/"
            ++ Inspect.Func.name(f)
            ++ ".dot";
          log("Fetching " ++ url);
          let _ =
            Lwt.bind(
              HttpClient.get("/" ++ url),
              dot => {
                dispatch @@ `Update_dot(dot);
                Lwt.return();
              },
            );
          ();
        }
      | _ => ()
      };
      None;
    },
    [|state.inspect|],
  );

  <div className="container-fluid">
    <div className="row">
      <div className="col-3 border-right"> <Sidebar state dispatch /> </div>
      <div className="col-9 d-flex flex-column vh-100">
        <Content state dispatch />
        <Panel state dispatch />
      </div>
    </div>
  </div>;
};
