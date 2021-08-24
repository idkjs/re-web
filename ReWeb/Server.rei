module Request: Request.S with type Reqd.t = Httpaf.Reqd.t;

module Service:
  Service.S with
    type Request.Reqd.t = Httpaf.Reqd.t and
    type Request.t('ctx) = Request.t('ctx);

module Filter:
  Filter.S with
    type Service.Request.Reqd.t = Httpaf.Reqd.t and
    type Service.Request.t('ctx) = Request.t('ctx) and
    type Service.t('ctx, 'resp) = Service.t('ctx, 'resp);

type path = list(string);

/** Pattern-matchable identifier of the request for routing purposes.
    Consists of:

    - HTTP method e.g. [`GET], [`POST]
    - [path] i.e. a list of path segments

    E.g., [GET /api/user/1] would be represented as
    [(`GET, ["api", "user", "1"])] */

type route = (Httpaf.Method.t, path);

/** A server is a function that takes a [route] and returns a service. A
    route is pattern-matchable (see above), so you will almost always do
    that to handle different endpoints with different services. */

type t('ctx, 'resp) = route => Service.t('ctx, 'resp);

/** [resource(?index, ?create, ?new_, ?edit, ?show, ?update, ?destroy)]
    returns a resource--that is a server--with the standard HTTP CRUD
    actions that you specify as services. The resource handles the paths
    and sets a reasonable cache policy corresponding to those CRUD
    actions:

    - [GET /scope] or [GET /scope/] calls [index]
    - [POST /scope] calls [create] and disables caching
    - [GET /scope/new] or [GET /scope/new/] calls [new_] and sets the
      response to cache publicly for a week
    - [GET /scope/id/edit] or [GET /scope/id/edit/] calls [edit id]
    - [GET /scope/id] or [GET /scope/id/] calls [show id]
    - [PATCH /scope/id] calls [update `PATCH id] and disables caching
    - [PUT /scope/id] calls [update `PUT id] and disables caching
    - [DELETE /scope/id] calls [destroy id] and disables caching

    The [scope] above is whatever scope you put the resource inside in a
    parent router, or maybe even the toplevel scope [/] if you use the
    resource directly as your toplevel server.

    All the service parameters are optional, with a 404 Not Found
    response as the default. Note that [resource] is just a convenience
    function; you can implement a custom resource yourself if needed, by
    creating a server function of type [('ctx, 'resp) t] instead. */

let resource:
  (
    ~index: Service.all('ctx)=?,
    ~create: Service.all('ctx)=?,
    ~new_: Service.all('ctx)=?,
    ~edit: string => Service.all('ctx)=?,
    ~show: string => Service.all('ctx)=?,
    ~update: ([ | `PATCH | `PUT], string) => Service.all('ctx)=?,
    ~destroy: string => Service.all('ctx)=?,
    route
  ) =>
  Service.all('ctx);

/** [serve(?port, server)] starts the top-level [server] listening on
    [port]. Top-level servers must have no context i.e. their context is
    [()]. */

let serve:
  (~port: int=?, t(unit, [ Response.http | Response.websocket])) => unit;
