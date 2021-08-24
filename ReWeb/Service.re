/** Please see here for API documentation. */
module type S = {
  module Config: ReWeb__Config.S;
  module Request: Request.S;

  /** A service is an asynchronous function from a request to a
      response. */

  type t('ctx, 'resp) = Request.t('ctx) => Lwt.t('resp);

  /** A type modelling services with an intersection of all response
      types. This type is most useful for services which set response
      headers, or come after filters which set response headers. */

  type all('ctx) = t('ctx, [ Response.http | Response.websocket]);
};

module Make = (R: Request.S) => {
  module Request = R;
  module Config = Request.Config;

  type t('ctx, 'resp) = Request.t('ctx) => Lwt.t('resp);
  type all('ctx) = t('ctx, [ Response.http | Response.websocket]);
};
