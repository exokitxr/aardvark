#include "aardvark_app_impl.h"
#include "aardvark_gadget_impl.h"
#include "aardvark/aardvark_server.h"

#include <algorithm>
#include <assert.h>

using namespace aardvark;

CAardvarkApp::CAardvarkApp( const std::string & sName, AvServerImpl *pParentServer )
	: m_sceneGraph( nullptr )
{
	static uint32_t s_uniqueId = 1;
	m_id = s_uniqueId++;

	m_sName = sName;
	m_pParentServer = pParentServer;
}


::kj::Promise<void> CAardvarkApp::destroy( DestroyContext context )
{
	m_pParentServer->removeApp( this );
	context.getResults().setSuccess( true );
	return kj::READY_NOW;
}

::kj::Promise<void> CAardvarkApp::name( NameContext context )
{
	context.getResults().setName( m_sName );
	return kj::READY_NOW;
}


::kj::Promise<void> CAardvarkApp::createGadget( CreateGadgetContext context )
{
	auto gadget = kj::heap<CAardvarkGadget>( context.getParams().getName(), this );
	auto& gadgetRef = *gadget;
	AvGadget::Client capability = kj::mv( gadget );

	context.getResults().setGadget( capability );

	gadgetRef.AddClient( capability );

	m_vecGadgets.push_back( &gadgetRef );

	return kj::READY_NOW;
}


::kj::Promise<void> CAardvarkApp::updateSceneGraph( UpdateSceneGraphContext context )
{
	m_sceneGraph = tools::newOwnCapnp( context.getParams().getRoot() );
	context.getResults().setSuccess( true );
	return kj::READY_NOW;
}


void CAardvarkApp::removeGadget( CAardvarkGadget *pGadget )
{
	auto iApp = std::find( m_vecGadgets.begin(), m_vecGadgets.end(), pGadget );
	if ( iApp != m_vecGadgets.end() )
	{
		pGadget->clearClients();
		m_vecGadgets.erase( iApp );
	}
}

void CAardvarkApp::gatherVisuals( AvVisuals_t & visuals )
{
	if ( m_sceneGraph.hasNodes() )
	{
		AvSceneGraphRoot_t root;
		root.root = m_sceneGraph;
		root.appId = m_id;
		visuals.vecSceneGraphs.push_back( root );
	}
	//for ( auto iGadget : m_vecGadgets )
	//{
	//	iGadget->gatherVisuals( visuals );
	//}
}

