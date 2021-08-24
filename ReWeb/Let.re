module Option = {
  let ( let* ) = Option.bind;
  let (let+) = (option, f) => Option.map(f, option);
};

module Result = {
  let ( let* ) = (result, f) =>
    switch (result) {
    | Ok(value) => f(value)
    | Error(e) => Error(e)
    };

  let (let+) = (result, f) =>
    switch (result) {
    | Ok(value) => Ok(f(value))
    | Error(e) => Error(e)
    };
};
