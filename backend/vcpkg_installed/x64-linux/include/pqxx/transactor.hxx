/* Transactor framework, a wrapper for safely retryable transactions.
 *
 * DO NOT INCLUDE THIS FILE DIRECTLY; include pqxx/transactor instead.
 *
 * Copyright (c) 2000-2026, Jeroen T. Vermeulen.
 *
 * See COPYING for copyright license.  If you did not receive a file called
 * COPYING with this source code, please notify the distributor of this
 * mistake, or contact the author.
 */
#ifndef PQXX_TRANSACTOR_HXX
#define PQXX_TRANSACTOR_HXX

#if !defined(PQXX_HEADER_PRE)
#  error "Include libpqxx headers as <pqxx/header>, not <pqxx/header.hxx>."
#endif

#include <functional>
#include <type_traits>

#include "pqxx/connection.hxx"
#include "pqxx/transaction.hxx"

namespace pqxx
{
/**
 * @defgroup transactor Transactor framework
 *
 * Sometimes a transaction can fail for completely transient reasons, such as a
 * conflict with another transaction in SERIALIZABLE isolation.  The right way
 * to handle those failures is often just to re-run the transaction from
 * scratch.
 *
 * For example, your REST API might be handling each HTTP request in its own
 * database transaction, and if this kind of transient failure happens, you
 * simply want to "replay" the whole request, in a fresh transaction.
 *
 * You won't necessarily want to execute the exact same SQL commands with the
 * exact same data.  Some of your SQL statements may depend on state that may
 * have changed by the time you replay the request.  For example, your code may
 * query information from the database that can change between retries.  So
 * instead of dumbly replaying the SQL, you re-run the same application code
 * that produced those SQL commands, from the start.
 *
 * The transactor framework makes it a little easier for you to do this safely,
 * and avoid typical pitfalls.  You encapsulate the work that you want to do
 * into a callable that you pass to the @ref perform function.
 *
 * Here's how it works.  You write your transaction code as a lambda or
 * function, which creates its own transaction object, does its work, and
 * commits at the end.  You pass that callback to @ref pqxx::perform, which
 * runs it for you.
 *
 * If there's a database-related failure inside your callback, libpqxx will
 * throw an exception.  Your transaction object goes out of scope and gets
 * destroyed, at which point it aborts implicitly.  Seeing this, @ref perform
 * tries running your callback again.  It may have to repeat this a few times.
 *
 * It stops trying that when...
 * * the callback succeeds; or
 * * it has failed too many times; or
 * * a non-libpqxx exception occurs; or
 * * there's an error that leaves the database in an unknown state.
 *
 * That last scenario can happen for example when you lose your network
 * connection to the database _just_ while you're waiting for it to confirm the
 * result of a commit.  In that situation, there's no way for the client to
 * know whether the transaction succeeded from the server's perspective (and so
 * was committed), or whether it failed and got aborted.  (If this possibility
 * is a major concern to you, also have a look at the @ref robusttransaction
 * class.  It does not help make your transaction more likely to succeed, but
 * it tries harder to avoid these unknown states.)
 *
 * The callback you pass to @ref perform takes no arguments.  If you're using
 * lambdas, the easy way to pass arguments is for the lambda to "capture" them
 * from your variables.
 *
 * Once your callback succeeds, it can return a result of your choosing, and
 * @ref perform will return that result back to you.
 *
 * @warning Transactors can be helpful, but they do require some extra care.
 * If your callback makes use of data from the database, you'll probably have
 * to query that data within your callback.  If there's a failure, and the
 * framework replays it, you'll be in a fresh transaction and the data in the
 * database may have changed under your feet.
 *
 * @warning Also be careful about changing variables or data structures from
 * within your callback.  The run may still fail, and perhaps get run again.
 * The ideal way to do it (in most cases) is to return your result from your
 * callback, and change your program's data state only after @ref perform
 * completes successfully.
 */
//@{

/// Simple way to execute a transaction with automatic retry.
/** @param callback Transaction code that can be called with no arguments.
 * @param attempts Maximum number of times to attempt performing @ref callback.
 *     Must be greater than zero.
 * @return Whatever your callback returns.
 */
template<typename TRANSACTION_CALLBACK>
inline std::invoke_result_t<TRANSACTION_CALLBACK> perform(
  TRANSACTION_CALLBACK &&callback, int attempts = 3, sl loc = sl::current())
{
  if (attempts <= 0)
    throw std::invalid_argument{
      "Zero or negative number of attempts passed to pqxx::perform()."};

  for (; attempts > 0; --attempts)
  {
    try
    {
      return std::invoke(callback);
    }
    catch (in_doubt_error const &)
    {
      // Not sure whether transaction went through or not.  The last thing in
      // the world that we should do now is try again!
      throw;
    }
    catch (statement_completion_unknown const &)
    {
      // Not sure whether our last statement succeeded.  Don't risk running it
      // again.
      throw;
    }
    catch (insufficient_resources const &)
    {
      // Server is being overloaded.  Back off.
      throw;
    }
    catch (too_many_connections const &)
    {
      // Server is being overloaded.  Back off.
      throw;
    }
    catch (failure const &)
    {
      // Some other database-related failure.  Retry, unless we've run out of
      // attempts.
      if (attempts <= 1)
        throw;
    }
  }
  throw internal_error{"Reached unreachable transactor state.", loc};
}


template<typename TRANSACTION_CALLBACK>
[[deprecated("No soruce_location needed.")]] inline std::invoke_result_t<
  TRANSACTION_CALLBACK>
perform(TRANSACTION_CALLBACK &&callback, sl loc)
{
  return perform(std::forward(callback), 3, loc);
}
} // namespace pqxx
//@}
#endif
