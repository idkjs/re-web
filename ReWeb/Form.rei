/** Use {!make} (see below) to create a www form that can be decoded to
    custom types using the form validation rules defined in fields. Use
    {!encode} to encode a value as a form. */;

/** A decoder is a function that takes an encoded value and returns a
    result of decoding the value. */

type decoder('a) = string => result('a, string);

/** Allows creating a list of form fields using normal list syntax with
    a local open (e.g. [Field.[bool("remember-me"), string("username")]] */

module Field: {
  /** A form field is a field name and a decoder function for the field. */

  type t('a);

  /** Used to create a form field list. */

  type list(_, _) =
    | []: list('a, 'a)
    | ::(t('a), list('b, 'c)): list('a => 'b, 'c); /***/

  /** [bool(name)] returns a boolean field named [name]. */

  let bool: string => t(bool);

  /** [ensure(pred, field)] returns a field that will succeed decoding a
      value [a] of type ['a] if [pred(a)] is [true]. Otherwise it will
      fail decoding. */

  let ensure: ('a => bool, t('a)) => t('a);

  /** [float(name)] returns a float field named [name]. */

  let float: string => t(float);

  /** [int(name)] returns an integer field named [name]. */

  let int: string => t(int);

  /** [make(name, decoder)] returns a field with a [decoder] of type
      ['a]. */

  let make: (string, decoder('a)) => t('a);

  /** [string(name)] returns a string field named [name]. */

  let string: string => t(string);
};

/** A web form is a list of fields and a 'constructor' that takes their
    decoded field values and returns a value of type ['ty]. */

type t('ctor, 'ty);

let decode:
  (t('ctor, 'ty), list((string, list(string)))) => result('ty, string);

/** [decoder(form)] takes a form definition (see above) and returns a
    decoder from that form to a type ['ty]. */

let decoder: t('ctor, 'ty) => decoder('ty);

/** [empty] is an empty form i.e. one that does not decode any form data
    and always succeeds at it. */

let empty: t(unit, unit);

/** [encode(fields, value)] is a query-encoded string form. It calls
    [fields value] to get the representation of [value] as a list of
    key-value string pairs. */

let encode: ('ty => list((string, string)), 'ty) => string;

/** [make(fields, ctor)] allows creating a form that can be used to
    decode (with {!decoder}) www forms. */

let make: (Field.list('ctor, 'ty), 'ctor) => t('ctor, 'ty);
