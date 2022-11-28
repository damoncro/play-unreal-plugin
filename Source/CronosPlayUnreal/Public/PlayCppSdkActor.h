// Copyright 2022, Cronos Labs. All Rights Reserved

/**
 * wallet-connect apis
 * how callback works for c++ events
  dynamic delegate is used for blueprint callback.
  (delegate, multicast delegate, event is for c++ only)
  when c++ callback is called, it calls sendEvent to trigger the delegate.
  ue4 async task is used for calling delegation.

  DECLARE_DYNAMIC_DELEGATE_OneParam(FWalletconnectSessionInfoDelegate, <-
  delegation name FWalletConnectSessionInfo, <- 1st parameter type SessionInfo);
  <- 1st parameter name

 */

#pragma once
#include "Async/Async.h" // clang-diagnostic-error: false positive, can be ignored
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayCppSdkLibrary/Include/extra-cpp-bindings/src/lib.rs.h"
#include "PlayCppSdkLibrary/Include/walletconnectcallback.h"
#include <mutex>
#include <queue>

#include "PlayCppSdkActor.generated.h"

/**
 Cronos Signed Transaction
 */
USTRUCT(BlueprintType)
struct FCronosSignedTransactionRaw {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  TArray<uint8> SignedTx;
};

// build signed-tx
DECLARE_DYNAMIC_DELEGATE_TwoParams(FCronosSignedTransactionDelegate,
                                   FCronosSignedTransactionRaw, TxResult,
                                   FString, Result);

/// wallet connect session state
UENUM(BlueprintType)
enum class EWalletconnectSessionState : uint8 {
  StateDisconnected UMETA(DisplayName = "Disconnected"),
  StateConnecting UMETA(DisplayName = "Connecting"),
  StateConnected UMETA(DisplayName = "Connected"),
  StateUpdated UMETA(DisplayName = "Updated")
};

UENUM(BlueprintType)
enum class EConnectionType : uint8 {
  URI_STRING UMETA(DisplayName = "Generate uri as String"),
  QR_TEXTURE UMETA(DisplayName = "Generate uri as a QR code 2D Texture"),
  LAUNCH_URL UMETA(DisplayName = "Launch uri with native wallet directly"),
};

/// wallet connect session info
USTRUCT(BlueprintType)
struct FWalletConnectSessionInfo {
  GENERATED_BODY()

  FWalletConnectSessionInfo();
  /// state
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  EWalletconnectSessionState sessionstate;

  /// if the wallet approved the connection
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  bool connected;
  /// hex-string(0x...), the accounts returned by the wallet
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  TArray<FString> accounts;
  /// u64, the chain id returned by the wallet
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString chain_id;
  /// the bridge server URL
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString bridge;
  /// the secret key used in encrypting wallet requests
  /// and decrypting wallet responses as per WalletConnect 1.0
  /// hex-string(0x...), 32 bytes
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString key;
  /// this is the client's randomly generated ID
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString client_id;
  /// json, the client metadata (that will be presented to the wallet in the
  /// initial request)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString client_meta;
  /// uuid, the wallet's ID
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString peer_id;
  /// json, the wallet's metadata
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString peer_meta;
  /// uuid, the one-time request ID
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString handshake_topic;
};
/// wallet connect callback
DECLARE_DYNAMIC_DELEGATE_OneParam(FWalletconnectSessionInfoDelegate,
                                  FWalletConnectSessionInfo, SessionInfo);

/// wallet connect address, 20 bytes
USTRUCT(BlueprintType)
struct FWalletConnectAddress {
  GENERATED_USTRUCT_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  TArray<uint8> address;
};

/// wallet connect session information
USTRUCT(BlueprintType)
struct FWalletConnectEnsureSessionResult {
  GENERATED_USTRUCT_BODY()
  FWalletConnectEnsureSessionResult()
      : addresses(TArray<FWalletConnectAddress>{}), chain_id(0) {}

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  TArray<FWalletConnectAddress> addresses;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  int64 chain_id;
};

/// wallet connect sign tx result
USTRUCT(BlueprintType)
struct FWalletSignTXEip155Result {
  GENERATED_USTRUCT_BODY()

  /// signature, 65 bytes, if successful
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  TArray<uint8> signature;

  /// error message, if successful, ""
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString result;
};

/// sign eip155 tx delegate
DECLARE_DYNAMIC_DELEGATE_OneParam(FWalletconnectSignEip155TransactionDelegate,
                                  FWalletSignTXEip155Result, SigningResult);

/// sign personal delegate
DECLARE_DYNAMIC_DELEGATE_OneParam(FWalletconnectSignPersonalDelegate,
                                  FWalletSignTXEip155Result, SigningResult);

/// initialize wallet connect delegate
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FInitializeWalletConnectDelegate,
                                     EConnectionType, connection_type, bool,
                                     Succeed, FString, message);

/// wallet connect ensure session delegate
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEnsureSessionDelegate,
                                   FWalletConnectEnsureSessionResult,
                                   SessionResult, FString, Result);

/// called when QR is ready
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnQRReady, UTexture2D *, Texture);

/// wallet connect eip155 tx information
USTRUCT(BlueprintType)
struct FWalletConnectTxEip155 {
  GENERATED_USTRUCT_BODY()
  /** hexstring, "0x..." */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString to;
  /** gas limit in decimal string */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString gas;
  /** gas price in decimal string */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString gas_price;
  /** decimal string, in wei units */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString value;
  /** data, as bytes */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  TArray<uint8> data;
  /** nonce in decimal string */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString nonce;
};

/// facade for wallet connect
UCLASS()
class CRONOSPLAYUNREAL_API APlayCppSdkActor : public AActor {
  GENERATED_BODY()

private:
  static ::com::crypto::game_sdk::WalletconnectClient *_coreClient;

  static const APlayCppSdkActor *_sdk;

  // Internal session result, it will be set after successfully calling
  // `EnsureSession`
  FWalletConnectEnsureSessionResult _session_result;

  /**
   * InitializeWalletConnect delegate, called after calling
   * `InitializeWalletConnect`
   */
  FInitializeWalletConnectDelegate OnInitializeWalletConnectDelegate;

  /**
   * EnsureSession delegate, called after calling `EnsureSession`
   */
  FEnsureSessionDelegate OnEnsureSessionDelegate;

public:
  static const APlayCppSdkActor *getInstance();

  // Sets default values for this actor's properties
  APlayCppSdkActor();

  ::com::crypto::game_sdk::WalletconnectClient *GetClient() const {
    return _coreClient;
  };

  void SetWalletConnectEnsureSessionResult(
      FWalletConnectEnsureSessionResult InWalletConnectEnsureSessionResult) {
    _session_result = InWalletConnectEnsureSessionResult;
  }

  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "GetWalletConnectEnsureSessionResult",
                    Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  FWalletConnectEnsureSessionResult GetWalletConnectEnsureSessionResult() {
    return _session_result;
  }

  const TArray<uint8> GetAddress() const {
    return _session_result.addresses[0].address;
  }
  const int64 GetChainId() const { return _session_result.chain_id; }

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;
  virtual void Destroyed() override;

public:
  void setCommon(com::crypto::game_sdk::WalletConnectTxCommon &common,
                 FString fromaddress, FString gaslimit, FString gasprice);
  /**
   *  Cronos rpc address
   *  for example , devnet:  http://127.0.0.1:8545
   * testnet: https://evm-dev-t3.cronos.org
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FString myCronosRpc;

  /**
   * Cronos chain-id
   * for example, devnet: 777
   * testnet: 338
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  int32 myCronosChainID;

  // Called every frame
  virtual void Tick(float DeltaTime) override;

  /**
   * destroy wallet-connect client
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "DestroyClient", Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void DestroyClient();

  /**
   * Connect wallet client with walletconnect (Only Crypto.com Defi Wallet is
   * suppported at this moment)
   * @param description wallet-connect client description
   * @param url wallet-connect server url
   * @param icon_urls wallet-connect icon urls
   * @param name wallet-connect name
   * @param chain_id the network chain id (if 0, retrived and decided by wallet,
   * if > 0, decided by the client)
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "ConnectWalletConnect",
                    Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void ConnectWalletConnect(FString description, FString url,
                            TArray<FString> icon_urls, FString name,
                            int64 chain_id, EConnectionType connection_type);

  /**
   * intialize wallet-connect client
   * @param description wallet-connect client description
   * @param url wallet-connect server url
   * @param icon_urls wallet-connect icon urls
   * @param name wallet-connect name
   * @param Out InitializeWalletConnect callback
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "InitializeWalletConnect",
                    Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void InitializeWalletConnect(
      FString description, FString url, TArray<FString> icon_urls, FString name,
      int64 chain_id, FInitializeWalletConnectDelegate Out,
      EConnectionType connection_type = EConnectionType::URI_STRING);

  UFUNCTION()
  void OnInitializeWalletConnect(EConnectionType connection_type, bool succeed,
                                 FString message);

  /**
   * Create session or restore ession, ensure session
   * @param Out EnsureSession callback
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "EnsureSession", Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void EnsureSession(FEnsureSessionDelegate Out);

  UFUNCTION()
  void OnNewSession(FWalletConnectEnsureSessionResult SessionResult,
                    FString Result);

  UFUNCTION()
  void OnRestoreSession(FWalletConnectEnsureSessionResult SessionResult,
                        FString Result);

  /**
   * Clear Session
   * @param success whether clearing session succeed or not
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "ClearSession", Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void ClearSession(bool &success);

  /**
   * setup callback to receive event
   * @param sessioninfodelegate callback to receive session info
   * @param success succeed or fail
   * @output_message  error message
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "SetupCallback", Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void
  SetupCallback(const FWalletconnectSessionInfoDelegate &sessioninfodelegate,
                bool &success, FString &output_message);

  /**
   * WalletConnect Session Information delegate, called after walletconnect
   * callback onConnected, onDisconnected, onConnecting, or onUpdated is called.
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FWalletconnectSessionInfoDelegate OnReceiveWalletconnectSessionInfoDelegate;

  /**
   * On QR Ready delegate, called after QR is ready
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayCppSdk")
  FOnQRReady OnQRReady;

  UFUNCTION()
  void OnWalletconnectSessionInfo(FWalletConnectSessionInfo SessionInfo);

  /**
   * get qr code string
   * @param output qr code string
   * @param success succeed or fail
   * @param output_message  error message
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "GetConnectionString",
                    Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void GetConnectionString(FString &output, bool &success,
                           FString &output_message);

  /**
   * get crypto wallet url
   * @param uri WalletConnect uri
   * @return url starts with cryptowallet://
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "GetCryptoWalletUrl",
                    Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  FString GetCryptoWalletUrl(FString uri);
  /**
   * save session information as string
   * @param output session information string
   * @param success succeed or fail
   * @param output_message  error message
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "SaveClient", Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void SaveClient(FString &output, bool &success, FString &output_message);

  /**
   * restore session information from string(json)
   * @param jsondata session information string(json)
   * @param success   succeed or fail
   * @param output_message  error message
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "RestoreClient", Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void RestoreClient(FString &jsondata, bool &success, FString &output_message);

  /**
   * sign general message
   * @param user_message user message to sign
   * @param signature signature byte arrays
   * @param success succeed or fail
   * @param output_message  error message
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "SignPersonal", Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void SignPersonal(FString user_message,
                    FWalletconnectSignPersonalDelegate Out);

  /**
   * sign EIP155 tx
   * @param info EIP 155 tx information
   * @param Out sign legacy tx result callback
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "SignEip155Transaction",
                    Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void SignEip155Transaction(FWalletConnectTxEip155 info,
                             FWalletconnectSignEip155TransactionDelegate Out);

  /**
   * send wallet-connect information to unreal game thread
   */
  void sendEvent(FWalletConnectSessionInfo) const;

  static void destroyCoreClient();

  /**
   * Transfers `token_id` token from `from_address` to `to_address`.
   * @param contractAddress erc721 contract
   * @param toAddress to address
   * @param tokenId token id
   * @param gasLimit gas limit
   * @param gasPrice gas price
   * @param Out FCronosSignedTransactionDelegate callback
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "Erc721TransferFrom",
                    Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void Erc721TransferFrom(FString contractAddress, FString toAddress,
                          FString tokenId, FString gasLimit, FString gasPrice,
                          FCronosSignedTransactionDelegate Out);

  /**
   * Gives permission to `approved_address` to transfer `token_id` token to
   another account.
   * The approval is cleared when the token is transferred. Only a single
   account can be
   * approved at a time, so approving the zero address clears previous
   approvals.
   * @param contractAddress erc721 contract
   * @param approvedAddress  address to approve
   * @param tokenId token id
   * @param gasLimit gas limit
   * @param gasPrice gas price
   * @param Out FCronosSignedTransactionDelegate callback
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "Erc721Approve", Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void Erc721Approve(FString contractAddress, FString approvedAddress,
                     FString tokenId, FString gasLimit, FString gasPrice,
                     FCronosSignedTransactionDelegate Out);

  /**
   * Transfers `amount` tokens of `token_id` from
   * `from_address` to `to_address` with `additional_data`
   * @param contractAddress erc1155 contract
   * @param toAddress to address
   * @param tokenId token id
   * @param amount amount
   * @param additionalData additional data
   * @param gasLimit gas limit
   * @param gasPrice gas price
   * @param Out FCronosSignedTransactionDelegate callback
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "Erc1155SafeTransferFrom",
                    Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void Erc1155SafeTransferFrom(FString contractAddress, FString toAddress,
                               FString tokenId, FString amount,
                               TArray<uint8> additionalData, FString gasLimit,
                               FString gasPrice,
                               FCronosSignedTransactionDelegate Out);

  /**
   * Enable or disable approval for a third party `approved_address` to manage
   * all of sender's assets
   * @param contractAddress erc1155 contract
   * @param approvedAddress address to approve
   * @param approved approved or not
   * @param gasLimit gas limit
   * @param gasPrice gas price
   * @param Out FCronosSignedTransactionDelegate callback
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "Erc1155Approve", Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void Erc1155Approve(FString contractAddress, FString approvedAddress,
                      bool approved, FString gasLimit, FString gasPrice,
                      FCronosSignedTransactionDelegate Out);

  /**
   * Moves `amount` tokens from the caller’s account to `to_address`.
   * @param contractAddress erc20 contract
   * @param toAddress to address
   * @param amount amount
   * @param gasLimit gas limit
   * @param gasPrice gas price
   * @param Out FCronosSignedTransactionDelegate callback
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "Erc20TransferFrom", Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void Erc20TransferFrom(FString contractAddress, FString toAddress,
                         FString amount, FString gasLimit, FString gasPrice,
                         FCronosSignedTransactionDelegate Out);

  /**
   * Allows `approved_address` to withdraw from your account multiple times,
   * up to the `amount` amount.
   * @param contractAddress erc20 contract
   * @param approvedAddress address to approve
   * @param amount amount
   * @param gasLimit gas limit
   * @param gasPrice gas price
   * @param Out FCronosSignedTransactionDelegate callback
   */
  UFUNCTION(BlueprintCallable,
            meta = (DisplayName = "Erc20Approve", Keywords = "PlayCppSdk"),
            Category = "PlayCppSdk")
  void Erc20Approve(FString contractAddress, FString approvedAddress,
                    FString amount, FString gasLimit, FString gasPrice,
                    FCronosSignedTransactionDelegate Out);
};
