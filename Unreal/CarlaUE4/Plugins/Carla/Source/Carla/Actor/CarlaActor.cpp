// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaActor.h" // 包含CARLA中Actor类的头文件
#include "Carla/OpenDrive/OpenDrive.h" // 包含OpenDrive地图解析器的头文件
#include "Carla/Util/NavigationMesh.h" // 包含导航网格的头文件
#include "Carla/Vehicle/CarlaWheeledVehicle.h" // 包含CARLA中轮式车辆的头文件
#include "Carla/Walker/WalkerController.h" // 包含行人控制器的头文件
#include "Carla/Walker/WalkerBase.h" // 包含行人基础类的头文件
#include "GameFramework/CharacterMovementComponent.h" // 包含角色移动组件的头文件
#include "Carla/Game/Tagger.h" // 包含标签器的头文件
#include "Carla/Vehicle/MovementComponents/CarSimManagerComponent.h" // 包含CarSim管理组件的头文件
#include "Carla/Vehicle/MovementComponents/ChronoMovementComponent.h" // 包含Chrono物理组件的头文件
#include "Carla/Traffic/TrafficLightBase.h" // 包含交通灯基础类的头文件
#include "Carla/Game/CarlaStatics.h" // 包含CARLA静态数据的头文件
#include "Components/CapsuleComponent.h" // 包含胶囊组件的头文件

#include <compiler/disable-ue4-macros.h> // 禁用UE4宏，防止与carla库的宏冲突
#include <carla/rpc/AckermannControllerSettings.h> // 包含Ackermann控制器设置的RPC结构
#include "carla/rpc/LabelledPoint.h" // 包含标记点的RPC结构
#include <carla/rpc/LightState.h> // 包含灯光状态的RPC结构
#include <carla/rpc/MapInfo.h> // 包含地图信息的RPC结构
#include <carla/rpc/MapLayer.h> // 包含地图层的RPC结构
#include <carla/rpc/VehicleAckermannControl.h> // 包含Ackermann控制的RPC结构
#include <carla/rpc/VehicleControl.h> // 包含车辆控制的RPC结构
#include <carla/rpc/VehiclePhysicsControl.h> // 包含车辆物理控制的RPC结构
#include <carla/rpc/VehicleLightState.h> // 包含车辆灯光状态的RPC结构
#include <carla/rpc/VehicleLightStateList.h> // 包含车辆灯光状态列表的RPC结构
#include <carla/rpc/WalkerBoneControlIn.h> // 包含行人骨骼控制输入的RPC结构
#include <carla/rpc/WalkerBoneControlOut.h> // 包含行人骨骼控制输出的RPC结构
#include <carla/rpc/WalkerControl.h> // 包含行人控制的RPC结构
#include <carla/rpc/VehicleWheels.h> // 包含车辆轮子信息的RPC结构
#include <carla/rpc/WeatherParameters.h> // 包含天气参数的RPC结构
#include <compiler/enable-ue4-macros.h> // 启用UE4宏

// FCarlaActor类的构造函数
FCarlaActor::FCarlaActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
    : TheActor(Actor),
      Info(std::move(Info)),
      Id(ActorId),
      State(InState),
      World(World)
{
    // 构造函数体内为空，初始化列表已经完成了所有成员变量的初始化
}

// FVehicleActor类的构造函数
FVehicleActor::FVehicleActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
    : FCarlaActor(ActorId, Actor, Info, InState, World) // 调用基类构造函数
{
  Type = ActorType::Vehicle; // 设置Actor类型为车辆
  ActorData = MakeShared<FVehicleData>();  // 创建车辆数据的共享指针
}

// FSensorActor类的构造函数
FSensorActor::FSensorActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
    : FCarlaActor(ActorId, Actor, Info, InState, World) // 调用基类构造函数
{
  // FSensorActor构造函数实现
}

    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
    : FCarlaActor(ActorId, Actor, Info, InState, World)// 调用基类构造函数
{
  Type = ActorType::Sensor; // 设置Actor类型为传感器
  ActorData = MakeShared<FActorSensorData>(); // 创建传感器数据的共享指针
}
// FTrafficSignActor类的构造函数
FTrafficSignActor::FTrafficSignActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
    : FCarlaActor(ActorId, Actor, Info, InState, World)// 调用基类构造函数
{
  Type = ActorType::TrafficSign; // 设置Actor类型为交通标志
  ActorData = MakeShared<FTrafficSignData>(); // 创建交通标志数据的共享指针
}
// FTrafficLightActor类的构造函数
FTrafficLightActor::FTrafficLightActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
    : FCarlaActor(ActorId, Actor, Info, InState, World)  // 调用基类构造函数
{
  Type = ActorType::TrafficLight;  // 设置Actor类型为交通灯
  ActorData = MakeShared<FTrafficLightData>();  // 创建交通灯数据的共享指针
}
// FWalkerActor类的构造函数
FWalkerActor::FWalkerActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
    : FCarlaActor(ActorId, Actor, Info, InState, World)
{
  Type = ActorType::Walker;// 设置Actor类型为行人
  ActorData = MakeShared<FWalkerData>(); // 创建行人数据的共享指针
}
// FOtherActor类的构造函数
FOtherActor::FOtherActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
    : FCarlaActor(ActorId, Actor, Info, InState, World)
{
  Type = ActorType::Other;  // 设置Actor类型为其他
  ActorData = MakeShared<FActorData>();  // 创建其他Actor数据的共享指针
}

// FCarlaActor类的静态函数，用于根据不同的Actor类型构造相应的FCarlaActor派生类实例
TSharedPtr<FCarlaActor> FCarlaActor::ConstructCarlaActor(
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      ActorType Type,
      carla::rpc::ActorState InState,
      UWorld* World)
{
 // 根据传入的Actor类型进行判断，并创建对应的派生类实例
  switch(Type)
  {
  case ActorType::TrafficSign:// 创建交通标志Actor的实例
    return MakeShared<FTrafficSignActor>(ActorId, Actor, std::move(Info), InState, World);
    break;
  case ActorType::TrafficLight: // 创建交通灯Actor的实例
    return MakeShared<FTrafficLightActor>(ActorId, Actor, std::move(Info), InState, World);
    break;
  case ActorType::Vehicle: // 创建车辆Actor的实例
    return MakeShared<FVehicleActor>(ActorId, Actor, std::move(Info), InState, World);
    break;
  case ActorType::Walker: // 创建行人Actor的实例
    return MakeShared<FWalkerActor>(ActorId, Actor, std::move(Info), InState, World);
    break;
  case ActorType::Sensor:// 创建传感器Actor的实例
    return MakeShared<FSensorActor>(ActorId, Actor, std::move(Info), InState, World);
    break;
  default: // 如果类型不匹配以上任何一种，则创建其他类型的Actor实例
    return MakeShared<FOtherActor>(ActorId, Actor, std::move(Info), InState, World);
    break;
  }
}

// Base FCarlaActor functions ---------------------

void FCarlaActor::PutActorToSleep(UCarlaEpisode* CarlaEpisode)
{
  State = carla::rpc::ActorState::Dormant;
  if (ActorData)
  {
    ActorData->RecordActorData(this, CarlaEpisode);
  }
  TheActor->Destroy();
  TheActor = nullptr;
}

void FCarlaActor::WakeActorUp(UCarlaEpisode* CarlaEpisode)
{
  TheActor = ActorData->RespawnActor(CarlaEpisode, *Info);
  if (TheActor == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Could not wake up dormant actor %d at location %s"), GetActorId(), *(ActorData->GetLocalTransform(CarlaEpisode).GetLocation().ToString()));
    return;
  }
  State = carla::rpc::ActorState::Active;
  ActorData->RestoreActorData(this, CarlaEpisode);
}

FTransform FCarlaActor::GetActorLocalTransform() const
{
 // 检查Actor是否处于休眠状态（Dormant）
if (IsDormant())
{
  // 如果Actor处于休眠状态，直接返回Actor的变换信息
  // ActorData->Rotation 表示Actor的旋转信息
  // ActorData->Location.ToFVector() 表示Actor的位置信息，转换为FVector类型
  // ActorData->Scale 表示Actor的缩放信息
  return FTransform(
      ActorData->Rotation,
      ActorData->Location.ToFVector(),
      ActorData->Scale);
}
else
{
  // 如果Actor不处于休眠状态，从Actor对象中获取其变换信息
  FTransform Transform = GetActor()->GetActorTransform();

  // 获取当前世界中的LargeMapManager对象，用于处理大地图逻辑
  ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(World);

  // 如果LargeMapManager对象存在
  if (LargeMap)
  {
    // 使用LargeMapManager将局部变换转换为全局变换
    Transform = LargeMap->LocalToGlobalTransform(Transform);
  }

  // 返回处理后的变换信息
  return Transform;
}

// FCarlaActor类中的成员函数，用于获取Actor的全局变换
FTransform FCarlaActor::GetActorGlobalTransform() const
{
  // 如果Actor处于休眠状态（Dormant），则直接返回Actor数据中的全局变换
  if (IsDormant())
  {
    // 返回一个FTransform对象，包含Actor的旋转、位置和缩放
    return FTransform(
        ActorData->Rotation,        // Actor的旋转
        ActorData->Location.ToFVector(), // Actor的位置，转换为FVector类型
        ActorData->Scale);         // Actor的缩放
  }
  else
  {
    // 如果Actor不处于休眠状态，则从Actor对象中获取其局部变换
    FTransform Transform = GetActor()->GetActorTransform();

    // 获取当前世界中的LargeMapManager对象，用于处理大地图逻辑
    ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(World);

    // 如果LargeMapManager对象存在，则使用它将局部变换转换为全局变换
    if (LargeMap)
    {
      Transform = LargeMap->LocalToGlobalTransform(Transform);
    }

    // 返回全局变换
    return Transform;
  }
}

FVector FCarlaActor::GetActorLocalLocation() const
{
  if (IsDormant())
  {
    FVector Location = ActorData->Location.ToFVector();
    ALargeMapManager* LargeMap =
        UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      Location = LargeMap->GlobalToLocalLocation(Location);
    }
    return Location;
  }
  else
  {
    return GetActor()->GetActorLocation();
  }
}

FVector FCarlaActor::GetActorGlobalLocation() const
{
  if (IsDormant())
  {
    return ActorData->Location.ToFVector();
  }
  else
  {
    FVector Location = GetActor()->GetActorLocation();
    ALargeMapManager* LargeMap =
        UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      Location = LargeMap->LocalToGlobalLocation(Location);
    }
    return Location;
  }
}

void FCarlaActor::SetActorLocalLocation(const FVector& Location, ETeleportType TeleportType)
{
  if (IsDormant())
  {
    FVector GlobalLocation = Location;
    ALargeMapManager* LargeMap =
        UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      GlobalLocation = LargeMap->LocalToGlobalLocation(GlobalLocation);
    }
    ActorData->Location = FDVector(GlobalLocation);
  }
  else
  {
    GetActor()->SetActorRelativeLocation(
        Location,
        false,
        nullptr,
        TeleportType);
  }
}

void FCarlaActor::SetActorGlobalLocation(
    const FVector& Location, ETeleportType TeleportType)
{
  if (IsDormant())
  {
    ActorData->Location = FDVector(Location);;
  }
  else
  {
    FVector LocalLocation = Location;
    ALargeMapManager* LargeMap =
        UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      LocalLocation = LargeMap->GlobalToLocalLocation(Location);
    }
    GetActor()->SetActorRelativeLocation(
        LocalLocation,
        false,
        nullptr,
        TeleportType);
  }
}

void FCarlaActor::SetActorLocalTransform(
    const FTransform& Transform, ETeleportType TeleportType)
{
  if (IsDormant())
  {
    FTransform GlobalTransform = Transform;
    ALargeMapManager* LargeMap =
        UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      GlobalTransform =
          LargeMap->LocalToGlobalTransform(GlobalTransform);
    }
    ActorData->Location = FDVector(GlobalTransform.GetLocation());
    ActorData->Rotation = GlobalTransform.GetRotation();
    ActorData->Scale = GlobalTransform.GetScale3D();
  }
  else
  {
    GetActor()->SetActorRelativeTransform(
        Transform,
        false,
        nullptr,
        TeleportType);
  }
}

void FCarlaActor::SetActorGlobalTransform(
    const FTransform& Transform, ETeleportType TeleportType)
{
  if (IsDormant())
  {
    ActorData->Location = FDVector(Transform.GetLocation());
    ActorData->Rotation = Transform.GetRotation();
    ActorData->Scale = Transform.GetScale3D();
  }
  else
  {
    FTransform LocalTransform = Transform;
    ALargeMapManager* LargeMap =
        UCarlaStatics::GetLargeMapManager(World);
    if (LargeMap)
    {
      LocalTransform =
          LargeMap->GlobalToLocalTransform(LocalTransform);
    }
    GetActor()->SetActorRelativeTransform(
        LocalTransform,
        false,
        nullptr,
        TeleportType);
  }
}

FVector FCarlaActor::GetActorVelocity() const
{
  if (IsDormant())
  {
    return ActorData->Velocity;
  }
  else
  {
    return GetActor()->GetVelocity();
  }
}

FVector FCarlaActor::GetActorAngularVelocity() const
{
  if (IsDormant())
  {
    return ActorData->AngularVelocity;
  }
  else
  {
    UPrimitiveComponent* Primitive =
        Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (Primitive)
    {
      return Primitive->GetPhysicsAngularVelocityInDegrees();
    }
  }
  return FVector();
}

ECarlaServerResponse FCarlaActor::SetActorTargetVelocity(const FVector& Velocity)
{
  if (IsDormant())
  {
    ActorData->Velocity = Velocity;
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->SetPhysicsLinearVelocity(
        Velocity,
        false,
        "None");
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::SetActorTargetAngularVelocity(const FVector& AngularVelocity)
{
  if (IsDormant())
  {
    ActorData->AngularVelocity = AngularVelocity;
  }
  else
  {
    UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->SetPhysicsAngularVelocityInDegrees(
        AngularVelocity,
        false,
        "None");
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::AddActorImpulse(const FVector& Impulse)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->AddImpulse(
        Impulse,
        "None",
        false);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::AddActorImpulseAtLocation(
    const FVector& Impulse, const FVector& Location)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }

    UE_LOG(LogCarla, Warning, TEXT("AddImpulseAtLocation: Experimental feature, use carefully."));

    RootComponent->AddImpulseAtLocation(
        Impulse,
        Location,
        "None");
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::AddActorForce(const FVector& Force)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->AddForce(
        Force,
        "None",
        false);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::AddActorForceAtLocation(
    const FVector& Force, const FVector& Location)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }

    UE_LOG(LogCarla, Warning, TEXT("AddForceAtLocation: Experimental feature, use carefully."));

    RootComponent->AddForceAtLocation(
        Force,
        Location,
        "None");
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::AddActorAngularImpulse(const FVector& AngularInpulse)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->AddAngularImpulseInDegrees(
        AngularInpulse,
        "None",
        false);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::AddActorTorque(const FVector& Torque)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->AddTorqueInDegrees(
        Torque,
        "None",
        false);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::SetActorSimulatePhysics(bool bEnabled)
{
  if (IsDormant())
  {
    ActorData->bSimulatePhysics = bEnabled;
  }
  else
  {
    // In the rest of actors, the physics is controlled with the UPrimitiveComponent, so we use
    // that for disable it.
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }

    RootComponent->SetSimulatePhysics(bEnabled);
    RootComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::SetActorCollisions(bool bEnabled)
{
  if (IsDormant())
  {
  }
  else
  {
    GetActor()->SetActorEnableCollision(bEnabled);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FCarlaActor::SetActorEnableGravity(bool bEnabled)
{
  if (IsDormant())
  {
  }
  else
  {
    auto RootComponent = Cast<UPrimitiveComponent>(GetActor()->GetRootComponent());
    if (RootComponent == nullptr)
    {
      return ECarlaServerResponse::FunctionNotSupported;
    }
    RootComponent->SetEnableGravity(bEnabled);
  }
  return ECarlaServerResponse::Success;
}

// FVehicleActor functions ---------------------

ECarlaServerResponse FVehicleActor::EnableActorConstantVelocity(const FVector& Velocity)
{
  if (IsDormant())
  {
  }
  else
  {
    auto CarlaVehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (CarlaVehicle == nullptr)
    {
      return ECarlaServerResponse::NullActor;
    }
    CarlaVehicle->ActivateVelocityControl(Velocity);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::DisableActorConstantVelocity()
{
  if (IsDormant())
  {
  }
  else
  {
    auto CarlaVehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (CarlaVehicle == nullptr)
    {
      return ECarlaServerResponse::NullActor;
    }
    CarlaVehicle->DeactivateVelocityControl();
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::GetPhysicsControl(FVehiclePhysicsControl& PhysicsControl)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    PhysicsControl = ActorData->PhysicsControl;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    PhysicsControl = Vehicle->GetVehiclePhysicsControl();
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::GetFailureState(carla::rpc::VehicleFailureState& FailureState)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    FailureState = ActorData->FailureState;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    FailureState = Vehicle->GetFailureState();
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::GetVehicleLightState(FVehicleLightState& LightState)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    LightState = ActorData->LightState;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }

    LightState = Vehicle->GetVehicleLightState();
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::OpenVehicleDoor(const EVehicleDoor DoorIdx)
{
  if (!IsDormant())
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    Vehicle->OpenDoor(DoorIdx);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::CloseVehicleDoor(const EVehicleDoor DoorIdx)
{
  if (!IsDormant())
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    Vehicle->CloseDoor(DoorIdx);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::ApplyPhysicsControl(
      const FVehiclePhysicsControl& PhysicsControl)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    ActorData->PhysicsControl = PhysicsControl;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }

    Vehicle->ApplyVehiclePhysicsControl(PhysicsControl);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::SetVehicleLightState(
    const FVehicleLightState& LightState)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    ActorData->LightState = LightState;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }

    Vehicle->SetVehicleLightState(LightState);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::SetWheelSteerDirection(
    const EVehicleWheelLocation& WheelLocation, float AngleInDeg)
{
  if (IsDormant())
  {
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if(Vehicle == nullptr){
      return ECarlaServerResponse::NotAVehicle;
    }
    Vehicle->SetWheelSteerDirection(WheelLocation, AngleInDeg);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::GetWheelSteerAngle(
      const EVehicleWheelLocation& WheelLocation, float& Angle)
{
  if (IsDormant())
  {
    Angle = 0;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if(Vehicle == nullptr){
      return ECarlaServerResponse::NotAVehicle;
    }

    Angle = Vehicle->GetWheelSteerAngle(WheelLocation);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::SetActorSimulatePhysics(bool bEnabled)
{
  if (IsDormant())
  {
    ActorData->bSimulatePhysics = bEnabled;
  }
  else
  {
    auto* CarlaVehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    // The physics in the vehicles works in a different way so to disable them.
    if (CarlaVehicle == nullptr){
      return ECarlaServerResponse::NotAVehicle;
    }
    CarlaVehicle->SetSimulatePhysics(bEnabled);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::ApplyControlToVehicle(
      const FVehicleControl& Control, const EVehicleInputPriority& Priority)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    ActorData->Control = Control;
    ActorData->bAckermannControlActive = false;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    Vehicle->ApplyVehicleControl(Control, Priority);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::ApplyAckermannControlToVehicle(
      const FVehicleAckermannControl& AckermannControl, const EVehicleInputPriority& Priority)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    ActorData->AckermannControl = AckermannControl;
    ActorData->bAckermannControlActive = true;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    Vehicle->ApplyVehicleAckermannControl(AckermannControl, Priority);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::GetVehicleControl(FVehicleControl& VehicleControl)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    VehicleControl = ActorData->Control;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    VehicleControl = Vehicle->GetVehicleControl();
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::GetVehicleAckermannControl(FVehicleAckermannControl& VehicleAckermannControl)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    VehicleAckermannControl = ActorData->AckermannControl;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    VehicleAckermannControl = Vehicle->GetVehicleAckermannControl();
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::GetAckermannControllerSettings(
  FAckermannControllerSettings& AckermannSettings)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    AckermannSettings = ActorData->AckermannControllerSettings;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    AckermannSettings = Vehicle->GetAckermannControllerSettings();
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::ApplyAckermannControllerSettings(
      const FAckermannControllerSettings& AckermannSettings)
{
  if (IsDormant())
  {
    FVehicleData* ActorData = GetActorData<FVehicleData>();
    ActorData->AckermannControllerSettings = AckermannSettings;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }

    Vehicle->ApplyAckermannControllerSettings(AckermannSettings);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::SetActorAutopilot(bool bEnabled, bool bKeepState)
{
  if (IsDormant())
  {
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::AutoPilotNotSupported;
    }
    auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (Controller == nullptr)
    {
      return ECarlaServerResponse::AutoPilotNotSupported;
    }
    Controller->SetAutopilot(bEnabled, bKeepState);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::GetVehicleTelemetryData(FVehicleTelemetryData& TelemetryData)
{
  if (IsDormant())
  {
    FVehicleTelemetryData EmptyTelemetryData;
    TelemetryData = EmptyTelemetryData;
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    TelemetryData = Vehicle->GetVehicleTelemetryData();
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::ShowVehicleDebugTelemetry(bool bEnabled)
{
  if (IsDormant())
  {
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    Vehicle->ShowDebugTelemetry(bEnabled);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::EnableCarSim(const FString& SimfilePath)
{
  if (IsDormant())
  {
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    UCarSimManagerComponent::CreateCarsimComponent(Vehicle, SimfilePath);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::UseCarSimRoad(bool bEnabled)
{
  if (IsDormant())
  {
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    auto* CarSimComponent = Vehicle->GetCarlaMovementComponent<UCarSimManagerComponent>();
    if(CarSimComponent)
    {
      CarSimComponent->UseCarSimRoad(bEnabled);
    }
    else
    {
      return ECarlaServerResponse::CarSimPluginNotEnabled;
    }
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::EnableChronoPhysics(
      uint64_t MaxSubsteps, float MaxSubstepDeltaTime,
      const FString& VehicleJSON, const FString& PowertrainJSON,
      const FString& TireJSON, const FString& BaseJSONPath)
{
  if (IsDormant())
  {
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    UChronoMovementComponent::CreateChronoMovementComponent(
        Vehicle,
        MaxSubsteps,
        MaxSubstepDeltaTime,
        VehicleJSON,
        PowertrainJSON,
        TireJSON,
        BaseJSONPath);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FVehicleActor::RestorePhysXPhysics()
{
  if (IsDormant())
  {
  }
  else
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(GetActor());
    if (Vehicle == nullptr)
    {
      return ECarlaServerResponse::NotAVehicle;
    }
    UBaseCarlaMovementComponent* MovementComponent = 
        Vehicle->GetCarlaMovementComponent<UBaseCarlaMovementComponent>();
    if(MovementComponent)
    {
      MovementComponent->DisableSpecialPhysics();
    }
  }
  return ECarlaServerResponse::Success;
}

// FSensorActor functions ---------------------

// FtrafficSignActor functions ---------------------

// FTrafficLightActor functions ---------------------

ECarlaServerResponse FTrafficLightActor::SetTrafficLightState(const ETrafficLightState& State)
{
  if (IsDormant())
  {
    FTrafficLightData* ActorData = GetActorData<FTrafficLightData>();
    ActorData->LightState = State;
  }
  else
  {
    auto TrafficLight = Cast<ATrafficLightBase>(GetActor());
    if (TrafficLight == nullptr)
    {
      return ECarlaServerResponse::NotATrafficLight;
    }
    TrafficLight->SetTrafficLightState(State);
  }
  return ECarlaServerResponse::Success;
}

ETrafficLightState FTrafficLightActor::GetTrafficLightState() const
{
  if (IsDormant())
  {
    const FTrafficLightData* ActorData = GetActorData<FTrafficLightData>();
    return ActorData->LightState;
  }
  else
  {
    auto TrafficLight = Cast<ATrafficLightBase>(GetActor());
    if (TrafficLight == nullptr)
    {
      return ETrafficLightState::Off;
    }
    return TrafficLight->GetTrafficLightState();
  }
}

UTrafficLightController* FTrafficLightActor::GetTrafficLightController()
{
  if (IsDormant())
  {
    FTrafficLightData* ActorData = GetActorData<FTrafficLightData>();
    return ActorData->Controller;
  }
  else
  {
    auto TrafficLight = Cast<ATrafficLightBase>(GetActor());
    if (TrafficLight == nullptr)
    {
      return nullptr;
    }
    return TrafficLight->GetTrafficLightComponent()->GetController();
  }
}

ECarlaServerResponse FTrafficLightActor::SetLightGreenTime(float time)
{
  if (IsDormant())
  {
    // Todo: implement
  }
  else
  {
    auto TrafficLight = Cast<ATrafficLightBase>(GetActor());
    if (TrafficLight == nullptr)
    {
      return ECarlaServerResponse::NotATrafficLight;
    }
    TrafficLight->SetGreenTime(time);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FTrafficLightActor::SetLightYellowTime(float time)
{
  if (IsDormant())
  {
    // Todo: implement
  }
  else
  {
    auto TrafficLight = Cast<ATrafficLightBase>(GetActor());
    if (TrafficLight == nullptr)
    {
      return ECarlaServerResponse::NotATrafficLight;
    }
    TrafficLight->SetYellowTime(time);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FTrafficLightActor::SetLightRedTime(float time)
{
  if (IsDormant())
  {
    // Todo: implement
  }
  else
  {
    auto TrafficLight = Cast<ATrafficLightBase>(GetActor());
    if (TrafficLight == nullptr)
    {
      return ECarlaServerResponse::NotATrafficLight;
    }
    TrafficLight->SetRedTime(time);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FTrafficLightActor::FreezeTrafficLight(bool bFreeze)
{
  if (IsDormant())
  {
    // Todo: implement
  }
  else
  {
    auto TrafficLight = Cast<ATrafficLightBase>(GetActor());
    if (TrafficLight == nullptr)
    {
      return ECarlaServerResponse::NotATrafficLight;
    }
    TrafficLight->SetTimeIsFrozen(bFreeze);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FTrafficLightActor::ResetTrafficLightGroup()
{
  if (IsDormant())
  {
    // Todo: implement
  }
  else
  {
    auto TrafficLight = Cast<ATrafficLightBase>(GetActor());
    if (TrafficLight == nullptr)
    {
      return ECarlaServerResponse::NotATrafficLight;
    }
    TrafficLight->GetTrafficLightComponent()->GetGroup()->ResetGroup();
  }
  return ECarlaServerResponse::Success;
}

// FWalkerActor functions ---------------------

ECarlaServerResponse FWalkerActor::SetWalkerState(
    const FTransform& Transform,
    carla::rpc::WalkerControl WalkerControl)
{
  FVector NewLocation = Transform.GetLocation();
  FVector CurrentLocation = GetActorGlobalLocation();

  // adjust position up by half of capsule height 
  // (because in Unreal walker is centered at the capsule middle,
  // while Recast uses the bottom point)
  UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(GetActor()->GetRootComponent());
  if (Capsule)
  {
    NewLocation.Z += Capsule->GetScaledCapsuleHalfHeight();
  }

  FTransform NewTransform = Transform;
  NewTransform.SetLocation(NewLocation);

  if (IsDormant())
  {
    FWalkerData* WalkerData = GetActorData<FWalkerData>();
    WalkerData->WalkerControl = WalkerControl;
  }
  else
  {
    auto * Walker = Cast<AWalkerBase>(GetActor());
    if (Walker && !Walker->bAlive)
    {
      return ECarlaServerResponse::WalkerDead;
    }

    // apply walker speed
    auto Pawn = Cast<APawn>(GetActor());
    if (Pawn == nullptr)
    {
      return ECarlaServerResponse::ActorTypeMismatch;
    }
    auto Controller = Cast<AWalkerController>(Pawn->GetController());
    if (Controller == nullptr)
    {
      return ECarlaServerResponse::WalkerIncompatibleController;
    }
    Controller->ApplyWalkerControl(WalkerControl);
  }
  SetActorGlobalTransform(NewTransform);
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FWalkerActor::GetWalkerControl(
    FWalkerControl& Control)
{
  if (IsDormant())
  {
    FWalkerData* WalkerData = GetActorData<FWalkerData>();
    Control = WalkerData->WalkerControl;
  }
  else
  {
    auto * Walker = Cast<AWalkerBase>(GetActor());
    if (Walker && !Walker->bAlive)
    {
      return ECarlaServerResponse::WalkerDead;
    }

    // apply walker speed
    auto Pawn = Cast<APawn>(GetActor());
    if (Pawn == nullptr)
    {
      return ECarlaServerResponse::ActorTypeMismatch;
    }
    auto Controller = Cast<AWalkerController>(Pawn->GetController());
    if (Controller == nullptr)
    {
      return ECarlaServerResponse::WalkerIncompatibleController;
    }
    Control = Controller->GetWalkerControl();
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FWalkerActor::SetActorSimulatePhysics(bool bEnabled)
{
  if (IsDormant())
  {
    ActorData->bSimulatePhysics = bEnabled;
  }
  else
  {
    auto* Character = Cast<ACharacter>(GetActor());
    // The physics in the walkers also works in a different way so to disable them,
    // we need to do it in the UCharacterMovementComponent.
    if (Character == nullptr)
    {
      return ECarlaServerResponse::NotAWalker;
    }
    auto CharacterMovement = Cast<UCharacterMovementComponent>(Character->GetCharacterMovement());
    if(bEnabled) {
      CharacterMovement->SetDefaultMovementMode();
    }
    else {
      CharacterMovement->DisableMovement();
    }
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FWalkerActor::SetActorEnableGravity(bool bEnabled)
{
  if (IsDormant())
  {
  }
  else
  {
    auto Character = Cast<ACharacter>(GetActor());
    // The physics in the walkers works in a different way so to disable them,
    // we need to do it in the UCharacterMovementComponent.
    if (Character == nullptr)
    {
      return ECarlaServerResponse::NotAWalker;
    }
    auto CharacterMovement = Cast<UCharacterMovementComponent>(Character->GetCharacterMovement());

    if(bEnabled) {
      CharacterMovement->SetDefaultMovementMode();
    }
    else {
      if (CharacterMovement->IsFlying() || CharacterMovement->IsFalling())
        CharacterMovement->DisableMovement();
    }
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FWalkerActor::ApplyControlToWalker(
    const FWalkerControl& Control)
{
  if (IsDormant())
  {
    FWalkerData* ActorData = GetActorData<FWalkerData>();
    ActorData->WalkerControl = Control;
  }
  else
  {
    auto Pawn = Cast<APawn>(GetActor());
    if (Pawn == nullptr)
    {
      return ECarlaServerResponse::NotAWalker;
    }
    auto Controller = Cast<AWalkerController>(Pawn->GetController());
    if (Controller == nullptr)
    {
      return ECarlaServerResponse::WalkerIncompatibleController;
    }
    Controller->ApplyWalkerControl(Control);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FWalkerActor::GetBonesTransform(FWalkerBoneControlOut& Bones)
{
  if (IsDormant())
  {
  }
  else
  {
    auto Pawn = Cast<APawn>(GetActor());
    if (Pawn == nullptr)
    {
      return ECarlaServerResponse::NotAWalker;
    }
    auto Controller = Cast<AWalkerController>(Pawn->GetController());
    if (Controller == nullptr)
    {
      return ECarlaServerResponse::WalkerIncompatibleController;
    }
    Controller->GetBonesTransform(Bones);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FWalkerActor::SetBonesTransform(const FWalkerBoneControlIn& Bones)
{
  if (IsDormant())
  {
  }
  else
  {
    auto Pawn = Cast<APawn>(GetActor());
    if (Pawn == nullptr)
    {
      return ECarlaServerResponse::NotAWalker;
    }
    auto Controller = Cast<AWalkerController>(Pawn->GetController());
    if (Controller == nullptr)
    {
      return ECarlaServerResponse::WalkerIncompatibleController;
    }
    Controller->SetBonesTransform(Bones);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FWalkerActor::BlendPose(float Blend)
{
  if (IsDormant())
  {
  }
  else
  {
    auto Pawn = Cast<APawn>(GetActor());
    if (Pawn == nullptr)
    {
      return ECarlaServerResponse::NotAWalker;
    }
    auto Controller = Cast<AWalkerController>(Pawn->GetController());
    if (Controller == nullptr)
    {
      return ECarlaServerResponse::WalkerIncompatibleController;
    }
    Controller->BlendPose(Blend);
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FWalkerActor::GetPoseFromAnimation()
{
  if (IsDormant())
  {
  }
  else
  {
    auto Pawn = Cast<APawn>(GetActor());
    if (Pawn == nullptr)
    {
      return ECarlaServerResponse::NotAWalker;
    }
    auto Controller = Cast<AWalkerController>(Pawn->GetController());
    if (Controller == nullptr)
    {
      return ECarlaServerResponse::WalkerIncompatibleController;
    }
    Controller->GetPoseFromAnimation();
  }
  return ECarlaServerResponse::Success;
}

ECarlaServerResponse FWalkerActor::SetActorDead()
{
  if (IsDormant())
  {
  }
  else
  {
    auto Pawn = Cast<APawn>(GetActor());
    if (Pawn == nullptr)
    {
      return ECarlaServerResponse::NotAWalker;
    }
    auto Walker = Cast<AWalkerBase>(Pawn);
    if (Walker == nullptr)
    {
      return ECarlaServerResponse::NotAWalker;
    }
    Walker->StartDeathLifeSpan();
    UE_LOG(LogCarla, Warning, TEXT("Walker starting life span by dead"));
  }
  return ECarlaServerResponse::Success;
}
