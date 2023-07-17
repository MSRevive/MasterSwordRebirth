#ifndef WRAPPER_CASCONTEXT_H
#define WRAPPER_CASCONTEXT_H

#include <angelscript.h>

/**
*	@defgroup ASContext Angelscript Context Utils
*
*	@{
*/

/**
*	A context that does not release the context in its destructor.
*/
class CASContext
{
public:
	/**
	*	Constructor. The context is not AddRef'd.
	*	@param context Context.
	*/
	CASContext( asIScriptContext& context )
		: m_pContext( &context )
	{
	}

	~CASContext() = default;

public:

	/**
	*	@return Whether this context is valid.
	*/
	operator bool() const { return m_pContext != nullptr; }

	asIScriptContext* GetContext() { return m_pContext; }

protected:
	asIScriptContext* m_pContext = nullptr;

private:
	CASContext( const CASContext& ) = delete;
	CASContext& operator=( const CASContext& ) = delete;
};

/**
*	RAII wrapper around a context that maintains ownership for its lifetime.
*/
class CASOwningContext final : public CASContext
{
public:
	/**
	*	Constructor. The context is AddRef'd.
	*	@param context Context.
	*/
	CASOwningContext( asIScriptContext& context )
		: CASContext( context )
	{
		m_pContext->AddRef();
	}

	/**
	*	Constructor. A context is acquired using asIScriptEngine::RequestContext.
	*	@param engine Script engine.
	*/
	CASOwningContext( asIScriptEngine& engine )
		: CASContext( *engine.RequestContext() )
	{
		m_pEngine = &engine;

		m_pEngine->AddRef();
	}

	/**
	*	Destructor.
	*/
	~CASOwningContext()
	{
		Release();
	}

	/**
	*	Releases the context.
	*/
	void Release();

	/**
	*	Releases ownership of the context.
	*/
	void ReleaseOwnership();

private:
	//TODO: consider: the engine can be retrieved using m_pContext->GetEngine. All that's needed is a flag that indicates where it came from.
	asIScriptEngine* m_pEngine = nullptr;
};

/** @} */

#endif //WRAPPER_CASCONTEXT_H