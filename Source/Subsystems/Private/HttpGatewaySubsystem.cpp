// HttpGatewaySubsystem.cpp

#include "HttpGatewaySubsystem.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "HttpPath.h"
#include "HttpServerResponse.h"   // FHttpServerResponse
#include "Async/Async.h"
#include "SlateNotificationsBFL.h"
#include "WorldAmbienceSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

/**
 * @brief Initialize HttpGateway module
 * @param Collection 
 */
void UHttpGatewaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("[HTTP] UHttpGatewaySubsystem Initialized"));

    FHttpServerModule& HttpServerModule = FHttpServerModule::Get();

    // bFailOnBindFailure = true : sans cela le module rend un router valide meme si le bind
    // a echoue, et on monterait la route sur un listener mort en annoncant un serveur qui ecoute.
    HttpRouter = HttpServerModule.GetHttpRouter(ListenPort, /*bFailOnBindFailure=*/true);

    if (!HttpRouter.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[HTTP] Invalid router on port %d"), ListenPort);

        USlateNotificationsBFL::SlateNotify(
            FText::FromString(FString::Printf(
                TEXT("[HTTP] Port %d deja utilise - une autre instance de l'editeur le detient ? ")
                TEXT("Les commandes de la regie n'arriveront pas."),
                ListenPort)),
            EMessageType::Error);

        return;
    }
    
    // Route principale
    FHttpPath Path(TEXT("/command"));

    const FHttpRequestHandler Handler =
        FHttpRequestHandler::CreateUObject(
            this,
            &UHttpGatewaySubsystem::HandleCommandRequest
        );

    CommandRouteHandle = HttpRouter->BindRoute(
        Path,
        EHttpServerRequestVerbs::VERB_GET | EHttpServerRequestVerbs::VERB_OPTIONS,
        Handler
    );

    if (!CommandRouteHandle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[HTTP] Unable to bind route /command"));

        USlateNotificationsBFL::SlateNotify(
            FText::FromString(FString::Printf(
                TEXT("[HTTP] Impossible de monter la route /command sur le port %d. ")
                TEXT("Les commandes de la regie n'arriveront pas."),
                ListenPort)),
            EMessageType::Error);

        HttpRouter.Reset();
        return;
    }

    // Log de demarrage uniquement quand le router est valide et la route reellement montee
    HttpServerModule.StartAllListeners();
    UE_LOG(LogTemp, Log, TEXT("[HTTP] Server started on http://127.0.0.1:%d"), ListenPort);
}

/**
 * @brief Deinitialize HttpGateway module
 */
void UHttpGatewaySubsystem::Deinitialize()
{
    if (HttpRouter.IsValid() && CommandRouteHandle.IsValid())
    {
        HttpRouter->UnbindRoute(CommandRouteHandle);
        CommandRouteHandle.Reset();
        HttpRouter.Reset();
    }

    Super::Deinitialize();
}

/**
 * @brief Handle the commands from the Web page
 * @param Request 
 * @param OnComplete 
 * @return 
 */
bool UHttpGatewaySubsystem::HandleCommandRequest(
        const FHttpServerRequest& Request,
        const FHttpResultCallback& OnComplete) const
{
    FString Group;
    FString Action;

    if (const FString* GroupStr = Request.QueryParams.Find(TEXT("group"))){
        Group = *GroupStr;
    }

    if (const FString* ActionStr = Request.QueryParams.Find(TEXT("action"))){
        Action = *ActionStr;
    }
    
    /*** RACE COMMANDS ***/
    if (Group.Equals(TEXT("race"), ESearchCase::IgnoreCase))
    {
        int64 idrace = INDEX_NONE;
        bool bDisplay = 0;
        
        // Move to race
        if (Action.Equals(TEXT("movetorace"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr); // "**" -> const TCHAR*
            }
            AsyncTask(ENamedThreads::GameThread,[this, idrace]()
            {
                OnMoveToRace.Broadcast(idrace);
            });
        }
        
        // Display Race
        if (Action.Equals(TEXT("displayrace"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr); // "**" -> const TCHAR*
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, bDisplay, idrace]()
            {
                OnDisplayRace.Broadcast(idrace, bDisplay);
            });
        }
        if (Action.Equals(TEXT("refresh"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr); // "**" -> const TCHAR*
            }
            AsyncTask(ENamedThreads::GameThread,[this, idrace]()
            {
                OnRefreshRace.Broadcast(idrace);
            });
        }
        
        // Display Path
        if (Action.Equals(TEXT("displaypath"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, bDisplay, idrace]()
            {
                OnDisplayPath.Broadcast(idrace, bDisplay);
            });
        }
        // Display Slope
        if (Action.Equals(TEXT("displayslope"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, bDisplay, idrace]
            {
                OnDisplaySlope.Broadcast(idrace, bDisplay);
            });
        }
        // Display Pulse
        if (Action.Equals(TEXT("displaypulse"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr); 
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0); 
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, bDisplay, idrace]()
            {
                OnDisplayPulse.Broadcast(idrace, bDisplay);
            });
        }
        if (Action.Equals(TEXT("displaykm"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, bDisplay, idrace]()
            {
                OnDisplayKms.Broadcast(idrace, bDisplay);
            });
        }
        // Select Teams in Race
        if (Action.Equals(TEXT("select"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, idrace]()
            {
                OnSelectRaceTeams.Broadcast(idrace);
            });
        }
        // Deselect Teams in Race
        if (Action.Equals(TEXT("deselect"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, idrace]()
            {
                OnDeselectRaceTeams.Broadcast(idrace);
            });
        }
        if (Action.Equals(TEXT("delete"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, idrace]()
            {
                OnDeleteRaceTeam.Broadcast(idrace);
            });
        }
        // FlyOver Play
        if (Action.Equals(TEXT("play"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, idrace]()
            {
                OnFlyForward.Broadcast(idrace);
            });
        }
        // FlyOver Pause
        if (Action.Equals(TEXT("pause"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, idrace]()
            {
                OnFlyPause.Broadcast(idrace);
            });
        }
        // FlyOver Stop
        if (Action.Equals(TEXT("stop"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, idrace]()
            {
                OnFlyStop.Broadcast(idrace);
            });
        }
        // FlyOver Play
        if (Action.Equals(TEXT("back"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**IdStr);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, idrace]()
            {
                OnFlyBackward.Broadcast(idrace);
            });
        }
    }
    
    /*** TEAMS COMMANDS ***/
    if (Group.Equals(TEXT("teams"), ESearchCase::IgnoreCase))
    {
        int64 idteam = INDEX_NONE;
        int64 idrace = INDEX_NONE;
        bool bDisplay = 0;
        // Display Team by TeamId ok
        if (Action.Equals(TEXT("displayteam"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idteam")))
            {
                idteam = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idteam, idrace, bDisplay]()
            {
                OnDisplayTeam.Broadcast(idteam, idrace, bDisplay);
            });
        }
        // Show/Hide teams ok
        if (Action.Equals(TEXT("displayallteams"), ESearchCase::IgnoreCase))
        {
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idrace, bDisplay]()
            {
                OnDisplayTeams.Broadcast(idrace, bDisplay);
            });
        }
        // Display Flag Team by TeamId ok
        if (Action.Equals(TEXT("displayflag"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idteam")))
            {
                idteam = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idteam, idrace, bDisplay]()
            {
                OnDisplayFlag.Broadcast(idteam, idrace, bDisplay);
            });
        }
        // Display Club Team by TeamId ok
        if (Action.Equals(TEXT("displayclub"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idteam")))
            {
                idteam = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, idteam, idrace, bDisplay]()
            {
                OnDisplayClub.Broadcast(idteam, idrace, bDisplay);
            });
        }
        // Display Photo Team by TeamId ok
        if (Action.Equals(TEXT("displayphoto"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idteam")))
            {
                idteam = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, idteam, idrace, bDisplay]()
            {
                OnDisplayPhoto.Broadcast(idteam, idrace, bDisplay);
            });
        }
        // Teleport on Team ok
        if (Action.Equals(TEXT("teleport"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idteam")))
            {
                idteam = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idteam, idrace]()
            {
                OnTeleportToTeam.Broadcast(idteam, idrace);
            });
        }
        // Follow Team
        if (Action.Equals(TEXT("follow"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idteam")))
            {
                idteam = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idteam, idrace]()
            {
                OnFollowTeam.Broadcast(idteam, idrace);
            });
        }
        // Stop Follow Team
        if (Action.Equals(TEXT("stopfollow"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idteam")))
            {
                idteam = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idteam, idrace]()
            {
                OnStopFollowTeam.Broadcast(idteam, idrace);
            });
        }
        // Animation autour d'une team
        if (Action.Equals(TEXT("animation"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idteam")))
            {
                idteam = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idteam, idrace]()
            {
                OnTeamAnimation.Broadcast(idteam, idrace);
            });
        }
        if (Action.Equals(TEXT("stopanimation"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idteam")))
            {
                idteam = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idteam, idrace]()
            {
                OnTeamStopAnimation.Broadcast(idteam, idrace);
            });
        }
    }
    
    /*** TEAMGROUPS COMMANDS ***/
    if (Group.Equals(TEXT("teamgroups"), ESearchCase::IgnoreCase))
    {
        int64 idteam = INDEX_NONE;
        int64 idgroup = INDEX_NONE;
        int64 idrace = INDEX_NONE;
        bool bDisplay = 0;
        
        // Add Team to Group
        if (Action.Equals(TEXT("assign"), ESearchCase::IgnoreCase))
        {
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idteam")))
            {
                idteam = FCString::Atoi64(**IdStr);
            }
            if (const FString* IdGroupStr = Request.QueryParams.Find(TEXT("idgroup")))
            {
                idgroup = FCString::Atoi64(**IdGroupStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idteam, idrace, idgroup]()
            {
                OnAddTeamToGroup.Broadcast(idrace, idteam, idgroup);
            });
        }
        // Remove Team from Group
        if (Action.Equals(TEXT("remove"), ESearchCase::IgnoreCase))
        {
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idteam")))
            {
                idteam = FCString::Atoi64(**IdStr);
            }
            if (const FString* IdGroupStr = Request.QueryParams.Find(TEXT("idgroup")))
            {
                idgroup = FCString::Atoi64(**IdGroupStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idrace, idteam, idgroup]()
            {
                OnRemoveTeamFromGroup.Broadcast(idrace, idteam, idgroup);
            });
        }
        // Toggle Group
        if (Action.Equals(TEXT("display"), ESearchCase::IgnoreCase))
        {
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* IdGroupStr = Request.QueryParams.Find(TEXT("idgroup")))
            {
                idgroup = FCString::Atoi64(**IdGroupStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0); 
            }
            AsyncTask(ENamedThreads::GameThread,[this, idrace, idgroup, bDisplay]()
            {
                OnToggleGroup.Broadcast(idrace, idgroup, bDisplay);
            });
        }
    }
    
    /*** CHECKPOINTS COMMANDS ***/
    if (Group.Equals(TEXT("checkpoints"), ESearchCase::IgnoreCase))
    {
        int64 idcheckpoint = INDEX_NONE;
        int64 idrace = INDEX_NONE;
        bool bDisplay = 0;
        
        // Toggle all checkpoints
        if (Action.Equals(TEXT("displayall"), ESearchCase::IgnoreCase))
        {
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idrace, bDisplay]()
            {
                OnDisplayCheckpoints.Broadcast(idrace, bDisplay);
            });
        }
        // Display Checkpoint
        if (Action.Equals(TEXT("display"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idcheckpoint")))
            {
                idcheckpoint = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idcheckpoint, idrace, bDisplay]()
            {
                OnDisplayCheckpoint.Broadcast(idcheckpoint, idrace, bDisplay);
            });
        }
        // Display Checkpoint Weather
        if (Action.Equals(TEXT("displayweather"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idcheckpoint")))
            {
                idcheckpoint = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idcheckpoint, idrace, bDisplay]()
            {
                OnDisplayCheckpointWeather.Broadcast(idcheckpoint, idrace, bDisplay);
            });
        }
        // Teleport to Checkpoint
        if (Action.Equals(TEXT("teleport"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idcheckpoint")))
            {
                idcheckpoint = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idcheckpoint, idrace]()
            {
                OnCheckpointTeleport.Broadcast(idcheckpoint, idrace);
            });
        }
        // Animation around Checkpoint
        if (Action.Equals(TEXT("animation"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idcheckpoint")))
            {
                idcheckpoint = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idcheckpoint, idrace]()
            {
                OnCheckpointAnimation.Broadcast(idcheckpoint, idrace);
            });
        }
        if (Action.Equals(TEXT("stopanimation"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idcheckpoint")))
            {
                idcheckpoint = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idcheckpoint, idrace]()
            {
                OnCheckpointStopAnimation.Broadcast(idcheckpoint, idrace);
            });
        }
    }
    
    /*** POIS GROUPS ***/
    if (Group.Equals(TEXT("pois"), ESearchCase::IgnoreCase))
    {
        int64 idpoi = INDEX_NONE;
        int64 idrace = INDEX_NONE;
        bool bDisplay = 0;
        
        // Display all pois
        if (Action.Equals(TEXT("displayall"), ESearchCase::IgnoreCase))
        {
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idrace, bDisplay]()
            {
                OnDisplayPois.Broadcast(idrace, bDisplay);
            });
        }
        // Display POI
        if (Action.Equals(TEXT("display"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idpoi")))
            {
                idpoi = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idpoi, idrace, bDisplay]()
            {
                OnDisplayPoi.Broadcast(idpoi, idrace, bDisplay);
            });
        }
        // Display POI Weather
        if (Action.Equals(TEXT("displayweather"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idpoi")))
            {
                idpoi = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            if (const FString* bDisplayStr = Request.QueryParams.Find(TEXT("show")))
            {
                bDisplay = (FCString::Atoi(**bDisplayStr) != 0);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idpoi, idrace, bDisplay]()
            {
                OnDisplayPoiWeather.Broadcast(idpoi, idrace, bDisplay);
            });
        }
        // Teleport to POI
        if (Action.Equals(TEXT("teleport"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idpoi")))
            {
                idpoi = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idpoi, idrace]()
            {
                OnPoiTeleport.Broadcast(idpoi, idrace);
            });
        }
        // Animation around POI
        if (Action.Equals(TEXT("animation"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idpoi")))
            {
                idpoi = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idpoi, idrace]()
            {
                OnPoiAnimation.Broadcast(idpoi, idrace);
            });
        }
        if (Action.Equals(TEXT("stopanimation"), ESearchCase::IgnoreCase))
        {
            if (const FString* IdStr = Request.QueryParams.Find(TEXT("idpoi")))
            {
                idpoi = FCString::Atoi64(**IdStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, idpoi, idrace]()
            {
                OnPoiStopAnimation.Broadcast(idpoi, idrace);
            });
        }
    }
    
    /*** SETTINGS COMMANDS ***/
    if (Group.Equals(TEXT("settings"), ESearchCase::IgnoreCase))
    {   
        float GlobalValue = 0.f;
        int64 idrace = INDEX_NONE;
        FString regieURL = FString();
        FString NewUrl = FString();
        
        if (Action.Equals(TEXT("fetch"), ESearchCase::IgnoreCase))
        {
            if (const FString* FetchValueStr = Request.QueryParams.Find(TEXT("value")))
            {
                GlobalValue = FCString::Atof(**FetchValueStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
                
            }
            AsyncTask(ENamedThreads::GameThread,[this, GlobalValue, idrace]()
            {
                OnUpdateFetch.Broadcast(GlobalValue, idrace);
            });
        }
        if (Action.Equals(TEXT("zoffset"), ESearchCase::IgnoreCase))
        {
            if (const FString* ZOffsetValueStr = Request.QueryParams.Find(TEXT("value")))
            {
                GlobalValue = FCString::Atof(**ZOffsetValueStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
                
            }
            AsyncTask(ENamedThreads::GameThread,[this, GlobalValue, idrace]()
            {
                OnUpdateOffset.Broadcast(GlobalValue, idrace);
            });
        }
        // Path Glow
        if (Action.Equals(TEXT("glow"), ESearchCase::IgnoreCase))
        {
            if (const FString* GlowValueStr = Request.QueryParams.Find(TEXT("value")))
            {
                GlobalValue = FCString::Atof(**GlowValueStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, GlobalValue, idrace]()
            {
                OnUpdateGlow.Broadcast(GlobalValue, idrace);
            });
        }
        if (Action.Equals(TEXT("pulseglow"), ESearchCase::IgnoreCase))
        {
            if (const FString* GlowValueStr = Request.QueryParams.Find(TEXT("value")))
            {
                GlobalValue = FCString::Atof(**GlowValueStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, GlobalValue, idrace]()
            {
                OnUpdatePulseGlow.Broadcast(GlobalValue, idrace);
            });
        }
        if (Action.Equals(TEXT("pulse"), ESearchCase::IgnoreCase))
        {
            if (const FString* PulseValueStr = Request.QueryParams.Find(TEXT("value")))
            {
                GlobalValue = FCString::Atof(**PulseValueStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, GlobalValue, idrace]()
            {
                OnUpdatePulse.Broadcast(GlobalValue, idrace);
            });
        }
        if (Action.Equals(TEXT("minmax"), ESearchCase::IgnoreCase))
        {
            float MinValue;
            float MaxValue;
            if (const FString* MinValueStr = Request.QueryParams.Find(TEXT("min")))
            {
                MinValue = FCString::Atof(**MinValueStr);
            }
            if (const FString* MaxValueStr = Request.QueryParams.Find(TEXT("max")))
            {
                MaxValue = FCString::Atof(**MaxValueStr);
            }
            if (const FString* idraceStr = Request.QueryParams.Find(TEXT("idrace")))
            {
                idrace = FCString::Atoi64(**idraceStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, MinValue, MaxValue, idrace]()
            {
                OnMinMaxUpdate.Broadcast(MinValue, MaxValue, idrace);
            });
        }
        if (Action.Equals(TEXT("nearfar"), ESearchCase::IgnoreCase))
        {
            float NearValue;
            float FarValue;
            if (const FString* NearValueStr = Request.QueryParams.Find(TEXT("near")))
            {
                NearValue = FCString::Atof(**NearValueStr);
            }
            if (const FString* FarValueStr = Request.QueryParams.Find(TEXT("far")))
            {
                FarValue = FCString::Atof(**FarValueStr);
            }
            
            AsyncTask(ENamedThreads::GameThread,[this, NearValue, FarValue]()
            {
                OnNearFarUpdate.Broadcast(NearValue, FarValue);
            });
        }
        if (Action.Equals(TEXT("mainurl"), ESearchCase::IgnoreCase))
        {
            if (const FString* urlValue = Request.QueryParams.Find(TEXT("url")))
            {
                NewUrl = **urlValue;
            }
            AsyncTask(ENamedThreads::GameThread,[this, NewUrl]()
            {
                OnMainUrlUpdate.Broadcast(NewUrl);
            });
        }
    }
    
    /*** CAMERA COMMANDS ***/
    if (Group.Equals(TEXT("camera"), ESearchCase::IgnoreCase))
    {
        float PitchValue = 0.f;
        float LengthValue = 0.f;
        float ZAnchorPositionValue = 0.f;
        if (Action.Equals(TEXT("pitch"), ESearchCase::IgnoreCase))
        {
            if (const FString* PitchValueStr = Request.QueryParams.Find(TEXT("value")))
            {
                PitchValue = FCString::Atof(**PitchValueStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, PitchValue]()
            {
                OnCameraAngleUpdated.Broadcast(PitchValue);
            });
        }
        if (Action.Equals(TEXT("distance"), ESearchCase::IgnoreCase))
        {
            if (const FString* LengthValueStr = Request.QueryParams.Find(TEXT("value")))
            {
                LengthValue = FCString::Atof(**LengthValueStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, LengthValue]()
            {
                OnArmLengthUpdated.Broadcast(LengthValue);
            });
        }
        if (Action.Equals(TEXT("anchor"), ESearchCase::IgnoreCase))
        {
            if (const FString* ZAnchorPositionValueStr = Request.QueryParams.Find(TEXT("value")))
            {
                ZAnchorPositionValue = FCString::Atof(**ZAnchorPositionValueStr);
            }
            AsyncTask(ENamedThreads::GameThread,[this, ZAnchorPositionValue]()
            {
                OnZAnchorUpdated.Broadcast(ZAnchorPositionValue);
            });
        }
    }
    
    
    /*** MONDE VIVANT COMMANDS ***/
    // group=monde&action=autoweather&show=1 : meteo reelle automatique
    // group=monde&action=citylights&show=1[&value=1.0] : lumieres des villes la nuit (et leur intensite)
    if (Group.Equals(TEXT("monde"), ESearchCase::IgnoreCase))
    {
        TArray<TPair<FString, FString>> Settings;
        const FString* ShowStr = Request.QueryParams.Find(TEXT("show"));
        const FString* ValueStr = Request.QueryParams.Find(TEXT("value"));
        const FString Show = (ShowStr && FCString::Atoi(**ShowStr) != 0) ? TEXT("1") : TEXT("0");

        if (Action.Equals(TEXT("autoweather"), ESearchCase::IgnoreCase) && ShowStr)
        {
            Settings.Emplace(TEXT("bAutoWeather"), Show);
        }
        if (Action.Equals(TEXT("citylights"), ESearchCase::IgnoreCase))
        {
            if (ShowStr) Settings.Emplace(TEXT("bCityLights"), Show);
            if (ValueStr && ValueStr->IsNumeric()) Settings.Emplace(TEXT("CityLightsIntensity"), *ValueStr);
        }
        // group=monde&action=fauna&show=1[&value=1.0] : oiseaux (et leur densite)
        if (Action.Equals(TEXT("fauna"), ESearchCase::IgnoreCase))
        {
            if (ShowStr) Settings.Emplace(TEXT("bFauna"), Show);
            if (ValueStr && ValueStr->IsNumeric()) Settings.Emplace(TEXT("FaunaDensity"), *ValueStr);
        }

        TWeakObjectPtr<UGameInstance> WeakGI(GetGameInstance());
        AsyncTask(ENamedThreads::GameThread, [WeakGI, Settings]()
        {
            UWorld* World = WeakGI.IsValid() ? WeakGI->GetWorld() : nullptr;
            UWorldAmbienceSubsystem* Ambience = World ? World->GetSubsystem<UWorldAmbienceSubsystem>() : nullptr;
            if (!Ambience) return;
            for (const TPair<FString, FString>& Setting : Settings)
            {
                Ambience->SetSettingByName(Setting.Key, Setting.Value);
            }
        });
    }

    // Empty Response
    const FString ResponseString = TEXT("");
    TUniquePtr<FHttpServerResponse> Response =
        FHttpServerResponse::Create(ResponseString, TEXT("text/plain"));

    // CORS pour toutes les réponses
    Response->Headers.FindOrAdd(TEXT("Access-Control-Allow-Origin")).Emplace(TEXT("*"));
    Response->Headers.FindOrAdd(TEXT("Access-Control-Allow-Methods")).Emplace(TEXT("GET, POST, OPTIONS"));
    Response->Headers.FindOrAdd(TEXT("Access-Control-Allow-Headers")).Emplace(TEXT("Content-Type"));
    
    if (Request.Verb == EHttpServerRequestVerbs::VERB_OPTIONS)
    {
        OnComplete(MoveTemp(Response));
        return true;
    }
    
    const FString Json = TEXT("{\"status\":\"ok\"}");
    Response = FHttpServerResponse::Create(Json, TEXT("application/json"));
    Response->Headers.FindOrAdd(TEXT("Access-Control-Allow-Origin")).Emplace(TEXT("*"));
    Response->Headers.FindOrAdd(TEXT("Access-Control-Allow-Methods")).Emplace(TEXT("GET, POST, OPTIONS"));
    Response->Headers.FindOrAdd(TEXT("Access-Control-Allow-Headers")).Emplace(TEXT("Content-Type"));

    // OnComplete attend un TUniquePtr<FHttpServerResponse>&&
    OnComplete(MoveTemp(Response));

    return true;
}
