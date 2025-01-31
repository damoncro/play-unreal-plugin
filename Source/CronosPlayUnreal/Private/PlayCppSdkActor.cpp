// Copyright 2022, Cronos Labs. All Rights Reserved
#include "PlayCppSdkActor.h" // clang-diagnostic-error: false postive, can be ignored
#include "Async/TaskGraphInterfaces.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "HAL/FileManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "PlayCppSdkBPLibrary.h"
#include "PlayCppSdkLibrary/Include/defi-wallet-core-cpp/src/lib.rs.h"
#include "PlayCppSdkLibrary/Include/extra-cpp-bindings/src/lib.rs.h"
#include "PlayCppSdkLibrary/Include/rust/cxx.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Utlis.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace rust;
using namespace com::crypto::game_sdk;

::com::crypto::game_sdk::WalletconnectClient *APlayCppSdkActor::_coreClient =
    NULL;

class UserWalletConnectCallback : public WalletConnectCallback {
  private:
    APlayCppSdkActor *PlayCppSdkPtr;
  public:
    UserWalletConnectCallback(APlayCppSdkActor *PlayCppSdkPtr);
    void onConnected(const WalletConnectSessionInfo &sessioninfo) const;
    void onDisconnected(const WalletConnectSessionInfo &sessioninfo) const;
    void onConnecting(const WalletConnectSessionInfo &sessioninfo) const;
    void onUpdated(const WalletConnectSessionInfo &sessioninfo) const;
};

UserWalletConnectCallback::UserWalletConnectCallback(
    APlayCppSdkActor *PlayCppSdk)
    : PlayCppSdkPtr(PlayCppSdk) {}

void UserWalletConnectCallback::onConnected( // NOLINT : flase positive, virtual
                                             // function cannot be static
    const WalletConnectSessionInfo &sessioninfo) const {
    UE_LOG(LogTemp, Log, TEXT("user c++ onConnected"));
    if (PlayCppSdkPtr) {
        APlayCppSdkActor *PlayCppSdk =
            PlayCppSdkPtr; // we need it to create a new varialbe for lambda
        // We could not Call PlayCppSdkPtr's function direclty, bcause it is not
        // in game thread. Instead, we can create AsyncTask and pass the
        // PlayCppSdkPtr and sessioninfo into it, then call the function in game
        // thread.
        AsyncTask(ENamedThreads::GameThread, [PlayCppSdk, sessioninfo]() {
            UE_LOG(LogTemp, Log, TEXT("Call GameThread in send event"));
            PlayCppSdk->OnReceiveWalletconnectSessionInfoDelegate
                .ExecuteIfBound(PlayCppSdk->SetWalletConnectSessionInfo(
                    sessioninfo, EWalletconnectSessionState::StateConnected));
        });
    } else {
        UE_LOG(LogTemp, Error, TEXT("Can not find PlayCppSdkActor onConnected"));
    }
}
void UserWalletConnectCallback::onDisconnected( // NOLINT : flase positive,
                                                // virtual function cannot be
                                                // static
    const WalletConnectSessionInfo &sessioninfo) const {
    UE_LOG(LogTemp, Log, TEXT("user c++ onDisconnected"));
    if (PlayCppSdkPtr) {
        APlayCppSdkActor *PlayCppSdk =
            PlayCppSdkPtr; // we need it to create a new varialbe for lambda
        // We could not Call PlayCppSdkPtr's function direclty, bcause it is not
        // in game thread. Instead, we can create AsyncTask and pass the
        // PlayCppSdkPtr and sessioninfo into it, then call the function in game
        // thread.
        AsyncTask(ENamedThreads::GameThread, [PlayCppSdk, sessioninfo]() {
            UE_LOG(LogTemp, Log, TEXT("Call GameThread in send event"));
            PlayCppSdk->OnReceiveWalletconnectSessionInfoDelegate
                .ExecuteIfBound(PlayCppSdk->SetWalletConnectSessionInfo(
                    sessioninfo,
                    EWalletconnectSessionState::StateDisconnected));
        });
    } else {
        UE_LOG(LogTemp, Error,
               TEXT("Can not find PlayCppSdkActor durig onDisconnected"));
    }
}
void UserWalletConnectCallback::
    onConnecting( // NOLINT : flase positive, virtual function cannot be static
        const WalletConnectSessionInfo &sessioninfo) const {
    UE_LOG(LogTemp, Log, TEXT("user c++ onConnecting"));
    if (PlayCppSdkPtr) {
        APlayCppSdkActor *PlayCppSdk =
            PlayCppSdkPtr; // we need it to create a new varialbe for lambda
        // We could not Call PlayCppSdkPtr's function direclty, bcause it is not
        // in game thread. Instead, we can create AsyncTask and pass the
        // PlayCppSdkPtr and sessioninfo into it, then call the function in game
        // thread.
        AsyncTask(ENamedThreads::GameThread, [PlayCppSdk, sessioninfo]() {
            UE_LOG(LogTemp, Log, TEXT("Call GameThread in send event"));
            PlayCppSdk->OnReceiveWalletconnectSessionInfoDelegate
                .ExecuteIfBound(PlayCppSdk->SetWalletConnectSessionInfo(
                    sessioninfo, EWalletconnectSessionState::StateConnecting));
        });
    } else {
        UE_LOG(LogTemp, Error,
               TEXT("Can not find PlayCppSdkActor durig onConnecting"));
    }
}
void UserWalletConnectCallback::onUpdated( // NOLINT : flase positive, virtual
                                           // function cannot be static
    const WalletConnectSessionInfo &sessioninfo) const {
    UE_LOG(LogTemp, Log, TEXT("user c++ onUpdated"));
    if (PlayCppSdkPtr) {
        APlayCppSdkActor *PlayCppSdk =
            PlayCppSdkPtr; // we need it to create a new varialbe for lambda
        // We could not Call PlayCppSdkPtr's function direclty, bcause it is not
        // in game thread. Instead, we can create AsyncTask and pass the
        // PlayCppSdkPtr and sessioninfo into it, then call the function in game
        // thread.
        AsyncTask(ENamedThreads::GameThread, [PlayCppSdk, sessioninfo]() {
            UE_LOG(LogTemp, Log, TEXT("Call GameThread in send event"));
            PlayCppSdk->OnReceiveWalletconnectSessionInfoDelegate
                .ExecuteIfBound(PlayCppSdk->SetWalletConnectSessionInfo(
                    sessioninfo, EWalletconnectSessionState::StateUpdated));
        });
    } else {
        UE_LOG(LogTemp, Error,
               TEXT("Can not find PlayCppSdkActor durig onUpdated"));
    }
}


// Sets default values
APlayCppSdkActor::APlayCppSdkActor() {
    // Set this actor to call Tick() every frame.  You can turn this off to
    // improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
    _coreClient = NULL;
}

// Called when the game starts or when spawned
void APlayCppSdkActor::BeginPlay() { Super::BeginPlay(); }

FWalletConnectSessionInfo APlayCppSdkActor::SetWalletConnectSessionInfo(
    ::com::crypto::game_sdk::WalletConnectSessionInfo src,
    EWalletconnectSessionState sessionstate) {
    FWalletConnectSessionInfo dst = FWalletConnectSessionInfo();
    dst.sessionstate = sessionstate;
    dst.connected = src.connected;
    dst.accounts.Empty();
    for (int i = 0; i < src.accounts.size(); i++) {
        FString account = UTF8_TO_TCHAR(src.accounts[i].c_str());
        dst.accounts.Add(account);
    }
    assert(src.accounts.size() == dst.accounts.Num());
    dst.chain_id = UTF8_TO_TCHAR(src.chain_id.c_str());
    dst.bridge = UTF8_TO_TCHAR(src.bridge.c_str());
    dst.key = UTF8_TO_TCHAR(src.key.c_str());
    dst.client_id = UTF8_TO_TCHAR(src.client_id.c_str());
    dst.client_meta = UTF8_TO_TCHAR(src.client_meta.c_str());
    dst.peer_id = UTF8_TO_TCHAR(src.peer_id.c_str());
    dst.peer_meta = UTF8_TO_TCHAR(src.peer_meta.c_str());
    dst.handshake_topic = UTF8_TO_TCHAR(src.handshake_topic.c_str());
    _session_info = dst; // set the internal session info
    return dst;
}
// Called every frame
void APlayCppSdkActor::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void APlayCppSdkActor::DestroyClient() { destroyCoreClient(); }

void APlayCppSdkActor::Destroyed() {
    Super::Destroyed();

    UE_LOG(LogTemp, Log, TEXT("PlayCppActor Destroyed"));
    DestroyClient();

    assert(NULL == _coreClient);
}

void APlayCppSdkActor::ConnectWalletConnect(FString description, FString url,
                                            TArray<FString> icon_urls,
                                            FString name, int64 chain_id,
                                            EConnectionType connection_type) {
    OnRestoreClientDelegate.BindDynamic(
        this, &APlayCppSdkActor::OnRestoreClientFinished);
    RestoreClient(OnRestoreClientDelegate);
    _description = description;
    _url = url;
    _icon_urls = icon_urls;
    _name = name;
    _chain_id = chain_id;
    _connection_type = connection_type;
}

void APlayCppSdkActor::InitializeWalletConnect(
    FString description, FString url, TArray<FString> icon_urls, FString name,
    int64 chain_id, FInitializeWalletConnectDelegate Out) {

    AsyncTask(
        ENamedThreads::AnyHiPriThreadNormalTask,
        [this, Out, description, url, icon_urls, name, chain_id]() {
            FWalletConnectEnsureSessionResult output;
            bool success = false;
            FString message;
            try {

                std::string mydescription = TCHAR_TO_UTF8(*description);
                std::string myurl = TCHAR_TO_UTF8(*url);
                Vec<String> myiconurls;
                for (int i = 0; i < icon_urls.Num(); i++) {
                    myiconurls.push_back(TCHAR_TO_UTF8(*icon_urls[i]));
                }

                std::string myname = TCHAR_TO_UTF8(*name);

                Box<WalletconnectClient> tmpClient = walletconnect_new_client(
                    mydescription, myurl, myiconurls, myname, (uint64)chain_id);
                _coreClient = tmpClient.into_raw();
                assert(_coreClient != NULL);

                success = true;

                switch (_connection_type) {
                case EConnectionType::URI_STRING:
                    // do nothing here, URI string is still needed to be
                    // got from `GetConnectionString` by users
                    break;
                case EConnectionType::QR_TEXTURE: {
                    FString GetConnectionStringOutput;
                    bool IsGetConnectionString;
                    FString GetConnectionStringOutputMessage;
                    GetConnectionString(GetConnectionStringOutput,
                                        IsGetConnectionString,
                                        GetConnectionStringOutputMessage);

                    if (IsGetConnectionString) {
                        UE_LOG(LogTemp, Log, TEXT("Connection String: %s"),
                               *GetConnectionStringOutput);

                        AsyncTask(ENamedThreads::GameThread,
                                  [this, GetConnectionStringOutput]() {
                                      UTexture2D *qr =
                                          UPlayCppSdkBPLibrary::GenerateQrCode(
                                              GetConnectionStringOutput);
                                      if (qr) {
                                          // Execute OnQRReady delagate, pass
                                          // the QR texture out
                                          this->OnQRReady.ExecuteIfBound(qr);
                                      } else {
                                      }
                                  });

                    } else {
                        UE_LOG(LogTemp, Error,
                               TEXT("Get Connection String failed: %s"),
                               *(GetConnectionStringOutputMessage));
                    }

                    break;
                }
                default:
                    break;
                }

            } catch (const std::exception &e) {

                success = false;
                message = FString::Printf(
                    TEXT("PlayCppSdk InitializeWalletConnect Error: %s"),
                    UTF8_TO_TCHAR(e.what()));
            }

            AsyncTask(ENamedThreads::GameThread, [Out, success, message]() {
                Out.ExecuteIfBound(success, message);
            });
        });
}

void APlayCppSdkActor::OnInitializeWalletConnectFinished(bool succeed,
                                                         FString message) {
    if (succeed) {
        UE_LOG(LogTemp, Log, TEXT("Initialize Wallet Connect succeeded"));

        // Setup Callback
        bool IsSetupCallback;
        FString SetupCallbackOutputMessage;
        OnSetupCallbackDelegate.BindDynamic(
            this, &APlayCppSdkActor::OnWalletconnectSessionInfo);
        SetupCallback(OnSetupCallbackDelegate, IsSetupCallback,
                      SetupCallbackOutputMessage);
        if (IsSetupCallback) {
            switch (_connection_type) {
            case EConnectionType::LAUNCH_URL: {
                FString GetConnectionStringOutput;
                bool IsGetConnectionString;
                FString GetConnectionStringOutputMessage;
                GetConnectionString(GetConnectionStringOutput,
                                    IsGetConnectionString,
                                    GetConnectionStringOutputMessage);
                if (IsGetConnectionString) {
                    UE_LOG(LogTemp, Log, TEXT("Connection String: %s"),
                           *GetConnectionStringOutput);

                    // Launch Crypto Wallet
                    UKismetSystemLibrary::LaunchURL(
                        GetCryptoWalletUrl(GetConnectionStringOutput));

                } else {
                    UE_LOG(LogTemp, Error,
                           TEXT("Get Connection String failed: %s"),
                           *(GetConnectionStringOutputMessage));
                    return;
                }

                break;
            }
            default:
                break;
            }

            UE_LOG(LogTemp, Display, TEXT("Setup Callbacked succeeded"));

            // Ensure session
            OnEnsureSessionDelegate.BindDynamic(
                this, &APlayCppSdkActor::OnNewSession);
            EnsureSession(OnEnsureSessionDelegate);

        } else {
            UE_LOG(LogTemp, Error, TEXT("Setup Callbacked failed: %s"),
                   *(SetupCallbackOutputMessage));
        }
    } else {
        UE_LOG(LogTemp, Error, TEXT("InitializeWalletConnect failed: %s"),
               *(message));
    }
}

void APlayCppSdkActor::RestoreClient(FRestoreClientDelegate Out) {
    AsyncTask(
        ENamedThreads::AnyHiPriThreadNormalTask,
        [this, Out]() {
            bool success;
            FString message;
            try {
                FString jsondata;
                success = FFileHelper::LoadFileToString(
                    jsondata,
                    *(FPaths::ProjectSavedDir() + "sessioninfo.json"));
                // if load file success
                // if session file is not empty
                if (success && !jsondata.IsEmpty()) {
                    std::string sessioninfostring = TCHAR_TO_UTF8(*jsondata);
                    Box<WalletconnectClient> tmpClient =
                        walletconnect_restore_client(sessioninfostring);
                    _coreClient = tmpClient.into_raw();
                    assert(_coreClient != NULL);

                    success = true;
                } else {
                    success = false;
                }

            } catch (const std::exception &e) {
                success = false;
                message =
                    FString::Printf(TEXT("PlayCppSdk RestoreClient Error: %s"),
                                    UTF8_TO_TCHAR(e.what()));
            }

            AsyncTask(ENamedThreads::GameThread, [Out, success, message]() {
                Out.ExecuteIfBound(success, message);
            });
        }

    );
}

void APlayCppSdkActor::OnRestoreClientFinished(bool succeed, FString message) {
    if (succeed) {
        UE_LOG(LogTemp, Log, TEXT("RestoreClient succeeded"));

        // Setup Callback
        bool IsSetupCallback;
        FString SetupCallbackOutputMessage;
        OnSetupCallbackDelegate.BindDynamic(
            this, &APlayCppSdkActor::OnWalletconnectSessionInfo);
        SetupCallback(OnSetupCallbackDelegate, IsSetupCallback,
                      SetupCallbackOutputMessage);
        if (IsSetupCallback) {
            // Ensure session
            OnEnsureSessionDelegate.BindDynamic(
                this, &APlayCppSdkActor::OnRestoreSession);
            EnsureSession(OnEnsureSessionDelegate);
        } else {
            UE_LOG(LogTemp, Error, TEXT("Setup Callbacked failed: %s"),
                   *(SetupCallbackOutputMessage));
        }

    } else {
        OnInitializeWalletConnectDelegate.BindDynamic(
            this, &APlayCppSdkActor::OnInitializeWalletConnectFinished);
        InitializeWalletConnect(_description, _url, _icon_urls, _name,
                                _chain_id, OnInitializeWalletConnectDelegate);
    }
}

void APlayCppSdkActor::EnsureSession(FEnsureSessionDelegate Out) {

    AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask, [this, Out]() {
        FWalletConnectEnsureSessionResult output;
        FString result;
        UE_LOG(LogTemp, Display, TEXT("EnsureSession..."));
        try {

            WalletConnectEnsureSessionResult sessionresult;
            if (_coreClient != NULL) {
                UE_LOG(LogTemp, Display, TEXT("EnsureSession blocking..."));
                sessionresult = _coreClient->ensure_session_blocking();
                UE_LOG(LogTemp, Display, TEXT("EnsureSession done..."));
                output.addresses.Empty();
                for (int i = 0; i < sessionresult.addresses.size(); i++) {
                    FWalletConnectAddress newaddress;
                    ::std::array<::std::uint8_t, 20> src =
                        sessionresult.addresses[i].address;

                    newaddress.address.Empty();
                    for (int j = 0; j < src.size(); j++) {
                        newaddress.address.Add(src[j]);
                    }
                    assert(20 == newaddress.address.Num());
                    output.addresses.Add(newaddress);
                    _session_info.accounts.Add(
                        UUtlis::ToHex(newaddress.address));
                }
                assert(output.addresses.Num() ==
                       sessionresult.addresses.size());
                output.chain_id = sessionresult.chain_id;

                _session_info.sessionstate =
                    EWalletconnectSessionState::StateRestored;
                _session_info.connected = true;
                _session_info.chain_id =
                    FString::FromInt(sessionresult.chain_id);

                SetWalletConnectEnsureSessionResult(output);
            } else {
                result = FString::Printf(
                    TEXT("PlayCppSdk EnsureSession Error Invalid Client"));
            }

        } catch (const std::exception &e) {
            result = FString::Printf(TEXT("PlayCppSdk EnsureSession Error: %s"),
                                     UTF8_TO_TCHAR(e.what()));
        }

        AsyncTask(ENamedThreads::GameThread, [Out, output, result]() {
            Out.ExecuteIfBound(output, result);
        });
    });
}

void APlayCppSdkActor::OnRestoreSession(
    FWalletConnectEnsureSessionResult SessionResult, FString Result) {
    UE_LOG(LogTemp, Log, TEXT("OnRestoreSession Result: %s"), *Result)
    if (SessionResult.addresses.Num() > 0) {
        UE_LOG(LogTemp, Log, TEXT("OnRestoreSession Account[0]: %s"),
               *UUtlis::ToHex(SessionResult.addresses[0].address));
    }
    UE_LOG(LogTemp, Log, TEXT("OnRestoreSession Chain id: %d"),
           SessionResult.chain_id);
    this->OnRestoreSessionReady.ExecuteIfBound(SessionResult, Result);
}

void APlayCppSdkActor::OnNewSession(
    FWalletConnectEnsureSessionResult SessionResult, FString Result) {
    UE_LOG(LogTemp, Log, TEXT("OnNewSession Result: %s"), *Result)
    // Only SaveClient if user approves. If users rejects, addresses would be
    // empty.
    if (SessionResult.addresses.Num() > 0) {
        UE_LOG(LogTemp, Log, TEXT("OnNewSession Account[0]: %s"),
               *UUtlis::ToHex(SessionResult.addresses[0].address));
        FString output;
        bool success;
        FString output_message;
        SaveClient(output, success, output_message);
    }
    UE_LOG(LogTemp, Log, TEXT("OnNewSession Chain id: %d"),
           SessionResult.chain_id);
    this->OnNewSessionReady.ExecuteIfBound(SessionResult, Result);
}

void APlayCppSdkActor::ClearSession(bool &success) {
    IFileManager &FileManager = IFileManager::Get();
    success =
        FileManager.Delete(*(FPaths::ProjectSavedDir() + "sessioninfo.json"));
    destroyCoreClient();
    FWalletConnectEnsureSessionResult session_result;
    _session_result = session_result;
    InitWalletconnectSessionState();
}

void APlayCppSdkActor::SetupCallback(
    const FWalletconnectSessionInfoDelegate &sessioninfodelegate, bool &success,
    FString &output_message) {
    try {
        if (NULL == _coreClient) {
            success = false;
            output_message = TEXT("Invalid Client");
            return;
        }

        // connect dynamic multicast delegate
        OnReceiveWalletconnectSessionInfoDelegate = sessioninfodelegate;

        // TODO, unsafe, pass the PlayCppSdk pointer from game thread to tokio
        // thread It could race
        WalletConnectCallback *usercallbackraw =
            new UserWalletConnectCallback(this);
        std::unique_ptr<WalletConnectCallback> usercallback(usercallbackraw);
        _coreClient->setup_callback_blocking(std::move(usercallback));

        success = true;
    } catch (const std::exception &e) {
        success = false;
        output_message =
            FString::Printf(TEXT("PlayCppSdk EnsureSession Error: %s"),
                            UTF8_TO_TCHAR(e.what()));
    }
}

void APlayCppSdkActor::OnWalletconnectSessionInfo(
    FWalletConnectSessionInfo SessionInfo) {
    switch (SessionInfo.sessionstate) {
    case EWalletconnectSessionState::StateDisconnected:
        bool success;
        this->ClearSession(success);
        if (success) {
            UE_LOG(LogTemp, Log, TEXT("sessioninfo.json was deleted"));

        } else {
            UE_LOG(LogTemp, Log,
                   TEXT("can not delete sessioninfo.json, please try again"));
        }
        break;
    default:
        break;
    }
}

void APlayCppSdkActor::GetConnectionString(FString &output, bool &success,
                                           FString &output_message) {
    try {
        if (NULL == _coreClient) {
            success = false;
            output_message = TEXT("Invalid Client");
            return;
        }
        String qrcode = _coreClient->get_connection_string();
        output = UTF8_TO_TCHAR(qrcode.c_str());

        success = true;
    } catch (const std::exception &e) {
        success = false;
        output_message = FString::Printf(TEXT("PlayCppSdk GetQrcode Error: %s"),
                                         UTF8_TO_TCHAR(e.what()));
    }
}

FString APlayCppSdkActor::GetCryptoWalletUrl(FString uri) {
#if PLATFORM_IOS
    // Use universal link on ios
    FString temp = FGenericPlatformHttp::UrlEncode(uri);
    FString url =
        FString::Printf(TEXT("https://wallet.crypto.com/wc?uri=%s"), *temp);
    return url;
#elif PLATFORM_ANDROID
    // Android simply needs uri
    return uri;
#else
    // Use deep link on other os
    FString temp = FGenericPlatformHttp::UrlEncode(uri);
    FString url = FString::Printf(TEXT("cryptowallet://wc?uri=%s"), *temp);
    return url;
#endif
}

void APlayCppSdkActor::SaveClient(FString &output, bool &success,
                                  FString &output_message) {
    try {
        if (NULL == _coreClient) {
            success = false;
            output_message = TEXT("Invalid Client");
            return;
        }

        String sessioninfo = _coreClient->save_client();
        output = UTF8_TO_TCHAR(sessioninfo.c_str());
        success = FFileHelper::SaveStringToFile(
            output, *(FPaths::ProjectSavedDir() + "sessioninfo.json"),
            FFileHelper::EEncodingOptions::ForceUTF8);
        UE_LOG(LogTemp, Log, TEXT("Saved sessioninfo.json to: %s"),
               *(FPaths::ProjectSavedDir() + "sessioninfo.json"));
    } catch (const std::exception &e) {
        success = false;
        output_message = FString::Printf(
            TEXT("PlayCppSdk SaveClient Error: %s"), UTF8_TO_TCHAR(e.what()));
    }
}

void copyTArrayToVec(const TArray<uint8> &src, Vec<uint8_t> &dst) {
    dst.clear();
    for (int i = 0; i < src.Num(); i++) {
        dst.push_back(src[i]);
    }
    assert(dst.size() == src.Num());
}
void copyVecToTArray(const Vec<uint8_t> &src, TArray<uint8> &dst) {
    dst.Empty();
    for (int i = 0; i < src.size(); i++) {
        dst.Add(src[i]);
    }
    assert(dst.Num() == src.size());
}

void APlayCppSdkActor::SignPersonal(FString user_message,
                                    FWalletconnectSignPersonalDelegate Out) {
    ::com::crypto::game_sdk::WalletconnectClient *coreclient = GetClient();
    // if no walletconnect session, return
    if (coreclient == nullptr)
        return;
    TArray<uint8> address = GetAddress();
    // if no address, return
    if (address.Num() == 0)
        return;
    AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask,
              [Out, coreclient, user_message, address, this]() {
                  FWalletSignTXEip155Result output;
                  try {

                      assert(20 == address.Num());
                      ::std::array<::std::uint8_t, 20> dstaddress;
                      for (int i = 0; i < address.Num(); i++) {
                          dstaddress[i] = address[i];
                      }
                      Vec<uint8_t> sig1 = _coreClient->sign_personal_blocking(
                          TCHAR_TO_UTF8(*user_message), dstaddress);

                      copyVecToTArray(sig1, output.signature);
                      assert(sig1.size() == output.Num());

                  } catch (const std::exception &e) {
                      output.result = FString::Printf(
                          TEXT("PlayCppSdk SignPersonal Error: %s"),
                          UTF8_TO_TCHAR(e.what()));
                  }

                  AsyncTask(ENamedThreads::GameThread,
                            [Out, output]() { Out.ExecuteIfBound(output); });
              });
}

void APlayCppSdkActor::SignEip155Transaction(
    FWalletConnectTxEip155 info,
    FWalletconnectSignEip155TransactionDelegate Out) {
    ::com::crypto::game_sdk::WalletconnectClient *coreclient = GetClient();
    // if no walletconnect session, return
    if (coreclient == nullptr)
        return;
    TArray<uint8> address = GetAddress();
    int64 chain_id = (uint64)GetChainId();
    // if no address, return
    if (address.Num() == 0 || chain_id == 0)
        return;
    AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask, [Out, coreclient,
                                                        address, chain_id, info,
                                                        this]() {
        FWalletSignTXEip155Result output;

        try {

            assert(20 == address.Num());
            ::std::array<::std::uint8_t, 20> dstaddress;
            for (int i = 0; i < address.Num(); i++) {
                dstaddress[i] = address[i];
            }

            WalletConnectTxEip155 myinfo;
            myinfo.to = TCHAR_TO_UTF8(*info.to);
            myinfo.common.gas_limit = TCHAR_TO_UTF8(*info.gas);
            myinfo.common.gas_price = TCHAR_TO_UTF8(*info.gas_price);
            myinfo.value = TCHAR_TO_UTF8(*info.value);
            copyTArrayToVec(info.data, myinfo.data);
            myinfo.common.nonce = TCHAR_TO_UTF8(*info.nonce);
            myinfo.common.chainid = chain_id;
            if (_coreClient != NULL) {

                Vec<uint8_t> sig1 =
                    _coreClient->sign_eip155_transaction_blocking(myinfo,
                                                                  dstaddress);

                copyVecToTArray(sig1, output.signature);
                assert(sig1.size() == output.Num());
            } else {
                output.result = FString::Printf(
                    TEXT("PlayCppSdk SignEip155Transaction Invalid Client"));
            }

        } catch (const std::exception &e) {
            output.result = FString::Printf(
                TEXT("PlayCppSdk SignEip155Transaction Error: %s"),
                UTF8_TO_TCHAR(e.what()));
        }

        AsyncTask(ENamedThreads::GameThread,
                  [Out, output]() { Out.ExecuteIfBound(output); });
    });
}

void APlayCppSdkActor::SendEip155Transaction(
    FWalletConnectTxEip155 info,
    FWalletconnectSendEip155TransactionDelegate Out) {
    ::com::crypto::game_sdk::WalletconnectClient *coreclient = GetClient();
    // if no walletconnect session, return
    if (coreclient == nullptr)
        return;
    TArray<uint8> address = GetAddress();
    int64 chain_id = (uint64)GetChainId();
    // if no address, return
    if (address.Num() == 0 || chain_id == 0)
        return;
    AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask, [Out, coreclient,
                                                        address, chain_id, info,
                                                        this]() {
        FWalletSendTXEip155Result output;

        try {

            assert(20 == address.Num());
            ::std::array<::std::uint8_t, 20> dstaddress;
            for (int i = 0; i < address.Num(); i++) {
                dstaddress[i] = address[i];
            }

            WalletConnectTxEip155 myinfo;
            myinfo.to = TCHAR_TO_UTF8(*info.to);
            myinfo.common.gas_limit = TCHAR_TO_UTF8(*info.gas);
            myinfo.common.gas_price = TCHAR_TO_UTF8(*info.gas_price);
            myinfo.value = TCHAR_TO_UTF8(*info.value);
            copyTArrayToVec(info.data, myinfo.data);
            myinfo.common.nonce = TCHAR_TO_UTF8(*info.nonce);
            myinfo.common.chainid = chain_id;
            if (_coreClient != NULL) {

                Vec<uint8_t> tx_hash =
                    _coreClient->send_eip155_transaction_blocking(myinfo,
                                                                  dstaddress);

                copyVecToTArray(tx_hash, output.tx_hash);
                assert(tx_hash.size() == output.Num());
            } else {
                output.result = FString::Printf(
                    TEXT("PlayCppSdk SendEip155Transaction Invalid Client"));
            }

        } catch (const rust::cxxbridge1::Error &e) {
            output.result = FString::Printf(
                TEXT("PlayCppSdk SendEip155Transaction Error: %s"),
                UTF8_TO_TCHAR(e.what()));
        }

        AsyncTask(ENamedThreads::GameThread,
                  [Out, output]() { Out.ExecuteIfBound(output); });
    });
}


void APlayCppSdkActor::destroyCoreClient() {
    if (_coreClient != NULL) {
        UE_LOG(LogTemp, Log, TEXT("PlayCppSdkActor destroyCoreClient"));

        // restored back, close tokio-runtime
        // ue4 editor gracefully closed
        // TODO crashed here? Likely to be fixed, but need to be careful
        Box<WalletconnectClient> tmpwallet =
            Box<WalletconnectClient>::from_raw(_coreClient);
        _coreClient = NULL;
    }
}

void StopWalletConnect() { APlayCppSdkActor::destroyCoreClient(); }

void APlayCppSdkActor::Erc721TransferFrom(
    FString contractAddress, FString toAddress, FString tokenId,
    FString gasLimit, FString gasPrice,
    FCronosSendContractTransactionDelegate Out) {

    TArray<uint8> fromAddressTArray = GetAddress();
    FString fromAddress = UUtlis::ToHex(fromAddressTArray);
    std::array<uint8_t, 20> fromAddressArray =
        UUtlis::ToArray(fromAddressTArray);
    // if no fromAddress, return
    if (fromAddress.IsEmpty())
        return;

    AsyncTask(
        ENamedThreads::AnyHiPriThreadNormalTask,
        [this, Out, contractAddress, fromAddress, toAddress, tokenId, gasLimit,
         gasPrice, fromAddressArray]() {
            FWalletSendTXEip155Result txresult;
            try {
                if (NULL == _coreClient) {
                    txresult.result = TEXT("Invalid Walletconnect");
                } else {
                    WalletConnectTxCommon common;
                    setCommon(common, gasLimit, gasPrice);
                    copyVecToTArray(_coreClient->send_contract_transaction(
                                        Erc721TransferFromAction(
                                            contractAddress, fromAddress,
                                            toAddress, tokenId),
                                        common, fromAddressArray),
                                    txresult.tx_hash);
                }

            } catch (const std::exception &e) {
                txresult.result = FString::Printf(
                    TEXT("CronosPlayUnreal Erc721TransferFrom Error: %s"),
                    UTF8_TO_TCHAR(e.what()));
            }

            AsyncTask(ENamedThreads::GameThread,
                      [Out, txresult]() { Out.ExecuteIfBound(txresult); });
        });
}

void APlayCppSdkActor::Erc721SafeTransferFrom(
    FString contractAddress, FString toAddress, FString tokenId,
    FString gasLimit, FString gasPrice,
    FCronosSendContractTransactionDelegate Out) {

    TArray<uint8> fromAddressTArray = GetAddress();
    FString fromAddress = UUtlis::ToHex(fromAddressTArray);
    std::array<uint8_t, 20> fromAddressArray =
        UUtlis::ToArray(fromAddressTArray);
    // if no fromAddress, return
    if (fromAddress.IsEmpty())
        return;

    AsyncTask(
        ENamedThreads::AnyHiPriThreadNormalTask,
        [this, Out, contractAddress, fromAddress, toAddress, tokenId, gasLimit,
         gasPrice, fromAddressArray]() {
            FWalletSendTXEip155Result txresult;
            try {
                if (NULL == _coreClient) {
                    txresult.result = TEXT("Invalid Walletconnect");
                } else {
                    WalletConnectTxCommon common;
                    setCommon(common, gasLimit, gasPrice);
                    copyVecToTArray(_coreClient->send_contract_transaction(
                                        Erc721SafeTransferFromAction(
                                            contractAddress, fromAddress,
                                            toAddress, tokenId),
                                        common, fromAddressArray),
                                    txresult.tx_hash);
                }

            } catch (const std::exception &e) {
                txresult.result = FString::Printf(
                    TEXT("CronosPlayUnreal Erc721SafeTransferFrom Error: %s"),
                    UTF8_TO_TCHAR(e.what()));
            }

            AsyncTask(ENamedThreads::GameThread,
                      [Out, txresult]() { Out.ExecuteIfBound(txresult); });
        });
}

void APlayCppSdkActor::Erc721SafeTransferFromWithAdditionalData(
    FString contractAddress, FString toAddress, FString tokenId,
    TArray<uint8> additionalData, FString gasLimit, FString gasPrice,
    FCronosSendContractTransactionDelegate Out) {

    TArray<uint8> fromAddressTArray = GetAddress();
    FString fromAddress = UUtlis::ToHex(fromAddressTArray);
    std::array<uint8_t, 20> fromAddressArray =
        UUtlis::ToArray(fromAddressTArray);
    // if no fromAddress, return
    if (fromAddress.IsEmpty())
        return;

    AsyncTask(
        ENamedThreads::AnyHiPriThreadNormalTask,
        [this, Out, contractAddress, fromAddress, toAddress, tokenId, gasLimit,
         gasPrice, fromAddressArray, additionalData]() {
            FWalletSendTXEip155Result txresult;
            try {
                if (NULL == _coreClient) {
                    txresult.result = TEXT("Invalid Walletconnect");
                } else {
                    WalletConnectTxCommon common;
                    setCommon(common, gasLimit, gasPrice);
                    copyVecToTArray(
                        _coreClient->send_contract_transaction(
                            Erc721SafeTransferFromWithAdditionalDataAction(
                                contractAddress, fromAddress, toAddress,
                                tokenId, additionalData),
                            common, fromAddressArray),
                        txresult.tx_hash);
                }

            } catch (const std::exception &e) {
                txresult.result = FString::Printf(
                    TEXT("CronosPlayUnreal "
                         "Erc721SafeTransferFromWithAdditionalData Error: %s"),
                    UTF8_TO_TCHAR(e.what()));
            }

            AsyncTask(ENamedThreads::GameThread,
                      [Out, txresult]() { Out.ExecuteIfBound(txresult); });
        });
}

void APlayCppSdkActor::Erc721Approve(
    FString contractAddress, FString approvedAddress, FString tokenId,
    FString gasLimit, FString gasPrice,
    FCronosSendContractTransactionDelegate Out) {

    TArray<uint8> fromAddressTArray = GetAddress();
    FString fromAddress = UUtlis::ToHex(fromAddressTArray);
    std::array<uint8_t, 20> fromAddressArray =
        UUtlis::ToArray(fromAddressTArray);
    // if no fromAddress, return
    if (fromAddress.IsEmpty())
        return;
    AsyncTask(
        ENamedThreads::AnyHiPriThreadNormalTask,
        [this, Out, contractAddress, fromAddress, approvedAddress, tokenId,
         gasLimit, gasPrice, fromAddressArray]() {
            FWalletSendTXEip155Result txresult;
            try {
                if (NULL == _coreClient) {
                    txresult.result = TEXT("Invalid Walletconnect");
                } else {
                    WalletConnectTxCommon common;
                    setCommon(common, gasLimit, gasPrice);
                    copyVecToTArray(
                        _coreClient->send_contract_transaction(
                            Erc721ApprovalAction(contractAddress,
                                                 approvedAddress, tokenId),
                            common, fromAddressArray),
                        txresult.tx_hash);
                }

            } catch (const std::exception &e) {
                txresult.result = FString::Printf(
                    TEXT("CronosPlayUnreal Erc721SafeTransferFrom Error: %s"),
                    UTF8_TO_TCHAR(e.what()));
            }

            AsyncTask(ENamedThreads::GameThread,
                      [Out, txresult]() { Out.ExecuteIfBound(txresult); });
        });
}

void APlayCppSdkActor::Erc721SetApprovalForAll(
    FString contractAddress, FString approvedAddress, bool approved,
    FString gasLimit, FString gasPrice,
    FCronosSendContractTransactionDelegate Out) {

    TArray<uint8> fromAddressTArray = GetAddress();
    FString fromAddress = UUtlis::ToHex(fromAddressTArray);
    std::array<uint8_t, 20> fromAddressArray =
        UUtlis::ToArray(fromAddressTArray);
    // if no fromAddress, return
    if (fromAddress.IsEmpty())
        return;
    AsyncTask(
        ENamedThreads::AnyHiPriThreadNormalTask,
        [this, Out, contractAddress, fromAddress, approvedAddress, gasLimit,
         gasPrice, fromAddressArray, approved]() {
            FWalletSendTXEip155Result txresult;
            try {
                if (NULL == _coreClient) {
                    txresult.result = TEXT("Invalid Walletconnect");
                } else {
                    WalletConnectTxCommon common;
                    setCommon(common, gasLimit, gasPrice);
                    copyVecToTArray(
                        _coreClient->send_contract_transaction(
                            Erc721SetApprovalForAllAction(
                                contractAddress, approvedAddress, approved),
                            common, fromAddressArray),
                        txresult.tx_hash);
                }

            } catch (const std::exception &e) {
                txresult.result = FString::Printf(
                    TEXT("CronosPlayUnreal Erc721SetApprovalForAll Error: %s"),
                    UTF8_TO_TCHAR(e.what()));
            }

            AsyncTask(ENamedThreads::GameThread,
                      [Out, txresult]() { Out.ExecuteIfBound(txresult); });
        });
}
void APlayCppSdkActor::Erc1155SafeTransferFrom(
    FString contractAddress, FString toAddress, FString tokenId, FString amount,
    TArray<uint8> additionalData, FString gasLimit, FString gasPrice,
    FCronosSendContractTransactionDelegate Out) {
    TArray<uint8> fromAddressTArray = GetAddress();
    FString fromAddress = UUtlis::ToHex(fromAddressTArray);
    std::array<uint8_t, 20> fromAddressArray =
        UUtlis::ToArray(fromAddressTArray);
    // if no fromAddress, return
    if (fromAddress.IsEmpty())
        return;
    AsyncTask(
        ENamedThreads::AnyHiPriThreadNormalTask,
        [this, Out, contractAddress, fromAddress, toAddress, tokenId, amount,
         additionalData, gasLimit, gasPrice, fromAddressArray]() {
            FWalletSendTXEip155Result txresult;
            try {
                if (NULL == _coreClient) {
                    txresult.result = TEXT("Invalid Walletconnect");
                } else {
                    WalletConnectTxCommon common;
                    setCommon(common, gasLimit, gasPrice);
                    copyVecToTArray(
                        _coreClient->send_contract_transaction(
                            Erc1155SafeTransferFromAction(
                                contractAddress, fromAddress, toAddress,
                                tokenId, amount, additionalData),
                            common, fromAddressArray),
                        txresult.tx_hash);
                }

            } catch (const std::exception &e) {
                txresult.result = FString::Printf(
                    TEXT("CronosPlayUnreal Erc1155SafeTransferFrom Error: %s"),
                    UTF8_TO_TCHAR(e.what()));
            }

            AsyncTask(ENamedThreads::GameThread,
                      [Out, txresult]() { Out.ExecuteIfBound(txresult); });
        });
}

void APlayCppSdkActor::Erc1155Approve(
    FString contractAddress, FString approvedAddress, bool approved,
    FString gasLimit, FString gasPrice,
    FCronosSendContractTransactionDelegate Out) {
    TArray<uint8> fromAddressTArray = GetAddress();
    FString fromAddress = UUtlis::ToHex(fromAddressTArray);
    std::array<uint8_t, 20> fromAddressArray =
        UUtlis::ToArray(fromAddressTArray);
    // if no fromAddress, return
    if (fromAddress.IsEmpty())
        return;
    AsyncTask(
        ENamedThreads::AnyHiPriThreadNormalTask,
        [this, Out, contractAddress, fromAddress, approvedAddress, approved,
         gasLimit, gasPrice, fromAddressArray]() {
            FWalletSendTXEip155Result txresult;
            try {
                if (NULL == _coreClient) {
                    txresult.result = TEXT("Invalid Walletconnect");
                } else {
                    WalletConnectTxCommon common;
                    setCommon(common, gasLimit, gasPrice);
                    copyVecToTArray(
                        _coreClient->send_contract_transaction(
                            Erc1155ApprovalAction(contractAddress,
                                                  approvedAddress, approved),
                            common, fromAddressArray),
                        txresult.tx_hash);
                }

            } catch (const std::exception &e) {
                txresult.result = FString::Printf(
                    TEXT("CronosPlayUnreal Erc1155Approve Error: %s"),
                    UTF8_TO_TCHAR(e.what()));
            }

            AsyncTask(ENamedThreads::GameThread,
                      [Out, txresult]() { Out.ExecuteIfBound(txresult); });
        });
}

void APlayCppSdkActor::Erc20Transfer(
    FString contractAddress, FString toAddress, FString amount,
    FString gasLimit, FString gasPrice,
    FCronosSendContractTransactionDelegate Out) {

    TArray<uint8> fromAddressTArray = GetAddress();
    FString fromAddress = UUtlis::ToHex(fromAddressTArray);
    std::array<uint8_t, 20> fromAddressArray =
        UUtlis::ToArray(fromAddressTArray);
    // if no fromAddress, return
    if (fromAddress.IsEmpty())
        return;
    AsyncTask(
        ENamedThreads::AnyHiPriThreadNormalTask,
        [this, Out, contractAddress, fromAddressArray, toAddress, amount,
         gasLimit, gasPrice]() {
            FWalletSendTXEip155Result txresult;
            try {
                if (NULL == _coreClient) {
                    txresult.result = TEXT("Invalid Walletconnect");
                } else {

                    WalletConnectTxCommon common;
                    setCommon(common, gasLimit, gasPrice);
                    copyVecToTArray(_coreClient->send_contract_transaction(
                                        Erc20TransferAction(contractAddress,
                                                            toAddress, amount),
                                        common, fromAddressArray),
                                    txresult.tx_hash);
                }

            } catch (const std::exception &e) {
                txresult.result = FString::Printf(
                    TEXT("CronosPlayUnreal Erc20Transfer Error: %s"),
                    UTF8_TO_TCHAR(e.what()));
            }

            AsyncTask(ENamedThreads::GameThread,
                      [Out, txresult]() { Out.ExecuteIfBound(txresult); });
        });
}

void APlayCppSdkActor::Erc20TransferFrom(
    FString contractAddress, FString toAddress, FString amount,
    FString gasLimit, FString gasPrice,
    FCronosSendContractTransactionDelegate Out) {

    TArray<uint8> fromAddressTArray = GetAddress();
    FString fromAddress = UUtlis::ToHex(fromAddressTArray);
    std::array<uint8_t, 20> fromAddressArray =
        UUtlis::ToArray(fromAddressTArray);
    // if no fromAddress, return
    if (fromAddress.IsEmpty())
        return;
    AsyncTask(
        ENamedThreads::AnyHiPriThreadNormalTask,
        [this, Out, contractAddress, fromAddress, toAddress, amount, gasLimit,
         gasPrice, fromAddressArray]() {
            FWalletSendTXEip155Result txresult;
            try {
                if (NULL == _coreClient) {
                    txresult.result = TEXT("Invalid Walletconnect");
                } else {

                    WalletConnectTxCommon common;
                    setCommon(common, gasLimit, gasPrice);
                    copyVecToTArray(_coreClient->send_contract_transaction(
                                        Erc20TransferFromAction(
                                            contractAddress, fromAddress,
                                            toAddress, amount),
                                        common, fromAddressArray),
                                    txresult.tx_hash);
                }

            } catch (const std::exception &e) {
                txresult.result = FString::Printf(
                    TEXT("CronosPlayUnreal Erc20TransferFrom Error: %s"),
                    UTF8_TO_TCHAR(e.what()));
            }

            AsyncTask(ENamedThreads::GameThread,
                      [Out, txresult]() { Out.ExecuteIfBound(txresult); });
        });
}

void APlayCppSdkActor::setCommon(WalletConnectTxCommon &common,
                                 FString gaslimit, FString gasprice) {
    // std::string mycronosrpc = TCHAR_TO_UTF8(*myCronosRpc);
    // common.web3api_url = mycronosrpc.c_str(); // uncessary
    common.web3api_url = "https://evm-dev-t3.cronos.org"; // uncessary, placeholder
    common.chainid = (uint64)GetChainId();
    common.gas_limit = TCHAR_TO_UTF8(*gaslimit);
    common.gas_price = TCHAR_TO_UTF8(*gasprice);
}

void APlayCppSdkActor::Erc20Approve(
    FString contractAddress, FString approvedAddress, FString amount,
    FString gasLimit, FString gasPrice,
    FCronosSendContractTransactionDelegate Out) {

    TArray<uint8> fromAddressTArray = GetAddress();
    FString fromAddress = UUtlis::ToHex(fromAddressTArray);
    std::array<uint8_t, 20> fromAddressArray =
        UUtlis::ToArray(fromAddressTArray);
    // if no fromAddress, return
    if (fromAddress.IsEmpty())
        return;
    AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask,
              [this, Out, contractAddress, fromAddress, approvedAddress, amount,
               gasLimit, gasPrice, fromAddressArray]() {
                  FWalletSendTXEip155Result txresult;
                  try {
                      if (NULL == _coreClient) {
                          txresult.result = TEXT("Invalid Walletconnect");
                      } else {
                          WalletConnectTxCommon common;
                          setCommon(common, gasLimit, gasPrice);
                          copyVecToTArray(
                              _coreClient->send_contract_transaction(
                                  Erc20ApprovalAction(contractAddress,
                                                      approvedAddress, amount),
                                  common, fromAddressArray),
                              txresult.tx_hash);
                      }

                  } catch (const std::exception &e) {
                      txresult.result = FString::Printf(
                          TEXT("CronosPlayUnreal Erc1155Approve Error: %s"),
                          UTF8_TO_TCHAR(e.what()));
                  }

                  AsyncTask(ENamedThreads::GameThread, [Out, txresult]() {
                      Out.ExecuteIfBound(txresult);
                  });
              });
}

rust::string APlayCppSdkActor::Erc20TransferAction(FString contract_address,
                                                   FString to_address,
                                                   FString amount) {
    // Construct the JSON object
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> FirstLevelObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> SecondLevelObject = MakeShareable(new FJsonObject);
    FirstLevelObject->SetObjectField("Erc20Transfer", SecondLevelObject);
    SecondLevelObject->SetStringField("contract_address", contract_address);
    SecondLevelObject->SetStringField("to_address", to_address);
    SecondLevelObject->SetStringField("amount", amount);
    RootObject->SetObjectField("ContractTransfer", FirstLevelObject);
    // Convert the JSON object to a string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    UE_LOG(LogTemp, Log, TEXT("Erc20TransferAction: %s"), *JsonString);

    return rust::string(TCHAR_TO_UTF8(*JsonString));
}

rust::string APlayCppSdkActor::Erc20TransferFromAction(FString contract_address,
                                                       FString from_address,
                                                       FString to_address,
                                                       FString amount) {
    // Construct the JSON object
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> FirstLevelObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> SecondLevelObject = MakeShareable(new FJsonObject);
    FirstLevelObject->SetObjectField("Erc20TransferFrom", SecondLevelObject);
    SecondLevelObject->SetStringField("contract_address", contract_address);
    SecondLevelObject->SetStringField("from_address", from_address);
    SecondLevelObject->SetStringField("to_address", to_address);
    SecondLevelObject->SetStringField("amount", amount);
    RootObject->SetObjectField("ContractTransfer", FirstLevelObject);
    // Convert the JSON object to a string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    return rust::string(TCHAR_TO_UTF8(*JsonString));
}

rust::string APlayCppSdkActor::Erc721TransferFromAction(
    FString contract_address, FString from_address, FString to_address,
    FString token_id) {
    // Construct the JSON object
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> FirstLevelObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> SecondLevelObject = MakeShareable(new FJsonObject);
    FirstLevelObject->SetObjectField("Erc721TransferFrom", SecondLevelObject);
    SecondLevelObject->SetStringField("contract_address", contract_address);
    SecondLevelObject->SetStringField("from_address", from_address);
    SecondLevelObject->SetStringField("to_address", to_address);
    SecondLevelObject->SetStringField("token_id", token_id);
    RootObject->SetObjectField("ContractTransfer", FirstLevelObject);
    // Convert the JSON object to a string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    return rust::string(TCHAR_TO_UTF8(*JsonString));
}

rust::string APlayCppSdkActor::Erc721SafeTransferFromAction(
    FString contract_address, FString from_address, FString to_address,
    FString token_id) {
    // Construct the JSON object
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> FirstLevelObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> SecondLevelObject = MakeShareable(new FJsonObject);
    FirstLevelObject->SetObjectField("Erc721SafeTransferFrom",
                                     SecondLevelObject);
    SecondLevelObject->SetStringField("contract_address", contract_address);
    SecondLevelObject->SetStringField("from_address", from_address);
    SecondLevelObject->SetStringField("to_address", to_address);
    SecondLevelObject->SetStringField("token_id", token_id);
    RootObject->SetObjectField("ContractTransfer", FirstLevelObject);
    // Convert the JSON object to a string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    return rust::string(TCHAR_TO_UTF8(*JsonString));
}

rust::string APlayCppSdkActor::Erc721SafeTransferFromWithAdditionalDataAction(
    FString contract_address, FString from_address, FString to_address,
    FString token_id, TArray<uint8> additional_data) {
    // Construct the JSON object
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> FirstLevelObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> SecondLevelObject = MakeShareable(new FJsonObject);
    FirstLevelObject->SetObjectField("Erc721SafeTransferFromWithAdditionalData",
                                     SecondLevelObject);
    SecondLevelObject->SetStringField("contract_address", contract_address);
    SecondLevelObject->SetStringField("from_address", from_address);
    SecondLevelObject->SetStringField("to_address", to_address);
    SecondLevelObject->SetStringField("token_id", token_id);
    // TODO is the hex string correct?
    SecondLevelObject->SetStringField("additional_data",
                                      UUtlis::ToHex(additional_data));
    RootObject->SetObjectField("ContractTransfer", FirstLevelObject);
    // Convert the JSON object to a string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    return rust::string(TCHAR_TO_UTF8(*JsonString));
}

rust::string APlayCppSdkActor::Erc1155SafeTransferFromAction(
    FString contract_address, FString from_address, FString to_address,
    FString token_id, FString amount, TArray<uint8> additional_data) {
    // Construct the JSON object
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> FirstLevelObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> SecondLevelObject = MakeShareable(new FJsonObject);
    FirstLevelObject->SetObjectField("Erc1155SafeTransferFrom",
                                     SecondLevelObject);
    SecondLevelObject->SetStringField("contract_address", contract_address);
    SecondLevelObject->SetStringField("from_address", from_address);
    SecondLevelObject->SetStringField("to_address", to_address);
    SecondLevelObject->SetStringField("token_id", token_id);
    SecondLevelObject->SetStringField("amount", amount);
    // TODO is the hex string correct?
    SecondLevelObject->SetStringField("additional_data",
                                      UUtlis::ToHex(additional_data));
    RootObject->SetObjectField("ContractTransfer", FirstLevelObject);
    // Convert the JSON object to a string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    return rust::string(TCHAR_TO_UTF8(*JsonString));
}

rust::string APlayCppSdkActor::Erc20ApprovalAction(FString contract_address,
                                                   FString approved_address,
                                                   FString amount) {
    // Construct the JSON object
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> FirstLevelObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> SecondLevelObject = MakeShareable(new FJsonObject);
    FirstLevelObject->SetObjectField("Erc20", SecondLevelObject);
    SecondLevelObject->SetStringField("contract_address", contract_address);
    SecondLevelObject->SetStringField("approved_address", approved_address);
    SecondLevelObject->SetStringField("amount", amount);
    RootObject->SetObjectField("ContractApproval", FirstLevelObject);
    // Convert the JSON object to a string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    return rust::string(TCHAR_TO_UTF8(*JsonString));
}

rust::string APlayCppSdkActor::Erc721ApprovalAction(FString contract_address,
                                                    FString approved_address,
                                                    FString token_id) {
    // Construct the JSON object
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> FirstLevelObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> SecondLevelObject = MakeShareable(new FJsonObject);
    FirstLevelObject->SetObjectField("Erc721Approve", SecondLevelObject);
    SecondLevelObject->SetStringField("contract_address", contract_address);
    SecondLevelObject->SetStringField("approved_address", approved_address);
    SecondLevelObject->SetStringField("token_id", token_id);
    RootObject->SetObjectField("ContractApproval", FirstLevelObject);
    // Convert the JSON object to a string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    return rust::string(TCHAR_TO_UTF8(*JsonString));
}

rust::string APlayCppSdkActor::Erc721SetApprovalForAllAction(
    FString contract_address, FString approved_address, bool approved) {
    // Construct the JSON object
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> FirstLevelObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> SecondLevelObject = MakeShareable(new FJsonObject);
    FirstLevelObject->SetObjectField("Erc721SetApprovalForAll",
                                     SecondLevelObject);
    SecondLevelObject->SetStringField("contract_address", contract_address);
    SecondLevelObject->SetStringField("approved_address", approved_address);
    SecondLevelObject->SetBoolField("approved", approved);
    RootObject->SetObjectField("ContractApproval", FirstLevelObject);
    // Convert the JSON object to a string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    return rust::string(TCHAR_TO_UTF8(*JsonString));
}

rust::string APlayCppSdkActor::Erc1155ApprovalAction(FString contract_address,
                                                     FString approved_address,
                                                     bool approved) {
    // Construct the JSON object
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> FirstLevelObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> SecondLevelObject = MakeShareable(new FJsonObject);
    FirstLevelObject->SetObjectField("Erc1155", SecondLevelObject);
    SecondLevelObject->SetStringField("contract_address", contract_address);
    SecondLevelObject->SetStringField("approved_address", approved_address);
    SecondLevelObject->SetBoolField("approved", approved);
    RootObject->SetObjectField("ContractApproval", FirstLevelObject);
    // Convert the JSON object to a string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    return rust::string(TCHAR_TO_UTF8(*JsonString));
}
