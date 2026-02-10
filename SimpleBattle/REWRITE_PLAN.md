# SimpleBattle Rewrite Plan (UE 5.7)

이 문서는 `SimpleBattle` 프로젝트의 현재 C++ 클래스들을 빠르게 재작성할 수 있도록, **현재 구조 분석**과 **재작성 설계/작업 순서**를 정리한다.

## 1. 프로젝트 스냅샷

- 엔진: UE **5.7** (`SimpleBattle/SimpleBattle.uproject`의 `EngineAssociation: 5.7`)
- 모듈: `SimpleBattle` (Runtime)
- 사용 모듈 의존성: `EnhancedInput`, `AIModule` 포함 (`SimpleBattle/Source/SimpleBattle/SimpleBattle.Build.cs`)
- 기본 맵: `/Game/Maps/BasicMap` (`SimpleBattle/Config/DefaultEngine.ini`)
- 주의: `DefaultEngine.ini` / `DefaultGame.ini`에 **GlobalDefaultGameMode** 지정이 보이지 않음.
  - 즉 `ASimpleGameMode`는 맵(World Settings)에서 오버라이드 되었을 가능성이 크다.

## 2. 현재 클래스 목록과 책임

### 게임 프레임

- `ASimpleGameMode` (`SimpleBattle/Source/SimpleBattle/Public/Game/SimpleGameMode.h`)
  - `DefaultPawnClass = ACharacterPlayer`
  - `PlayerControllerClass = ASimplePlayerController`
  - `BeginPlay()`에서 테스트용 적 1기 하드코딩 스폰

- `ASimplePlayerController` (`SimpleBattle/Source/SimpleBattle/Public/Player/SimplePlayerController.h`)
  - 마우스 커서 표시 + `FInputModeGameAndUI` 설정

### 플레이어

- `ACharacterPlayer` (`SimpleBattle/Source/SimpleBattle/Public/Character/CharacterPlayer.h`)
  - Top-down 카메라(스프링암+카메라)
  - `EnhancedInput` 바인딩: Move/Attack/Jump
  - Tick에서 마우스 커서 위치로 캐릭터 회전
  - 외부 의존: `UInputDataAsset`로 IMC/IA를 주입받는 구조

- `UInputDataAsset` (`SimpleBattle/Source/SimpleBattle/Public/Input/InputDataAsset.h`)
  - `DefaultMappingContext`, `MoveAction`, `AttackAction`, `JumpAction`를 묶는 Primary Data Asset

### 적/AI

- `ACharacterEnemy` (`SimpleBattle/Source/SimpleBattle/Public/Character/CharacterEnemy.h`)
  - AIController를 `AEnemyAIController`로 지정
  - 공격 경고 텔레그래프: `UAttackWarningComponent` 위임
  - 외형: `UStaticMeshAppearanceComponent` (빨간색으로 설정)

- `AEnemyAIController` (`SimpleBattle/Source/SimpleBattle/Public/AI/EnemyAIController.h`)
  - 상태머신: Idle -> Moving -> AttackWarning -> Idle
  - `MoveToActor(Player, AttackRange*0.8)`
  - AttackWarning 동안 플레이어를 바라보도록 회전 보간
  - Timer로 상태 전환 (IdleDuration, WarningDuration)

### 비주얼 컴포넌트

- `UStaticMeshAppearanceComponent` (`SimpleBattle/Source/SimpleBattle/Public/Component/StaticMeshAppearanceComponent.h`)
  - 런타임에 `Cone`, `Sphere` 기본 메시를 로드해서 체스 폰 모양 구성
  - **머티리얼을 코드로 생성**하고 `UMaterialInstanceDynamic`으로 색상 파라미터 제어
  - 중요한 제약: 머티리얼 그래프 연결은 `#if WITH_EDITOR`로 가드됨

- `UAttackWarningComponent` (`SimpleBattle/Source/SimpleBattle/Public/Component/AttackWarningComponent.h`)
  - 런타임에 `UDecalComponent`를 NewObject로 만들고 등록
  - `RevealProgress` 스칼라 파라미터로 와이프 애니메이션(전방 스윕) 구현
  - 중요한 제약: 기본 머티리얼을 코드로 생성하며, 핵심 그래프 연결은 `#if WITH_EDITOR`로 가드됨

## 3. 현재 구조의 핵심 흐름(런타임 시나리오)

1. 맵 로딩: `/Game/Maps/BasicMap`
2. (맵/프로젝트 설정에 따라) GameMode가 `ASimpleGameMode`로 설정되어 있다면:
   - 기본 폰: `ACharacterPlayer`
   - PC: `ASimplePlayerController` (마우스 표시)
   - `BeginPlay()`에서 `ACharacterEnemy` 1기 스폰
3. 적은 `AEnemyAIController`가 오토 포제스:
   - Idle 타이머 -> 이동 -> 사거리 진입 -> 경고(데칼 표시) -> Idle 복귀
4. 플레이어는 Tick에서 커서 방향으로 회전, Enhanced Input으로 이동/점프

## 4. 현재 코드의 리스크/갭(재작성 시 우선 해결)

### 4.1 입력 설정이 “기본값만으로”는 동작하지 않을 가능성

- `ACharacterPlayer::InputDataAsset`는 `EditDefaultsOnly`인데, 코드에서 기본값을 지정하지 않는다.
- GameMode가 C++ `ACharacterPlayer`를 기본 폰으로 쓰면, 별도의 BP 디폴트 설정이 없어서
  - `InputDataAsset` 미할당 -> `SetupPlayerInputComponent()`에서 에러 로그 -> 입력 바인딩 불가
- 재작성 방향:
  - (권장) `BP_PlayerCharacter`를 만들고 거기에 `InputDataAsset` 지정 후 GameMode가 BP를 기본 폰으로 사용
  - 또는 C++에서 `ConstructorHelpers::FObjectFinder`로 디폴트 에셋을 지정

### 4.2 GameMode 설정 경로가 불명확

- `DefaultEngine.ini`에 기본 맵은 있지만 GlobalDefaultGameMode가 보이지 않는다.
- 현재 `ASimpleGameMode`는 맵의 World Settings에서 설정되었을 가능성이 크다.
- 재작성 방향:
  - 프로젝트 차원 기본 GameMode를 `DefaultGame.ini`에 명시하거나
  - 기본 맵 `BasicMap`의 World Settings를 명확히 관리(문서화 포함)

### 4.3 머티리얼을 “코드 생성 + WITH_EDITOR 그래프 연결”에 의존

- `UStaticMeshAppearanceComponent`, `UAttackWarningComponent` 모두:
  - 에디터에서는 보이지만, 패키징/런타임 빌드에서 그래프 연결이 빠질 수 있음
- 재작성 방향(권장):
  - 머티리얼/머티리얼 인스턴스는 **Content 에셋으로** 만들고, C++은 파라미터만 제어
  - 컴포넌트는 `UMaterialInterface*`를 `EditDefaultsOnly`로 받아서 `UMaterialInstanceDynamic::Create()`만 수행

### 4.4 텔레그래프 데칼 생성 방식

- `UAttackWarningComponent`는 런타임에 `UDecalComponent`를 NewObject로 생성/등록한다.
- 재작성 시 더 안전한 형태:
  - `USceneComponent` 기반으로 바꾸고, `UDecalComponent`를 **DefaultSubobject**로 보유(수명/등록/세이브/에디터 노출이 쉬움)

## 5. 재작성 목표(권장 스코프)

- 플레이어:
  - Top-down 이동 + 커서 바라보기 유지
  - 공격 입력(AttackAction)은 최소한의 “피드백/쿨다운/히트 판정”까지 구현
- 적:
  - 단순 상태머신 유지(가벼운 샘플 프로젝트 목적)
  - AttackWarning 텔레그래프와 실제 공격(대미지) 분리
- 비주얼:
  - 코드 생성 머티리얼 제거, 에셋 기반으로 전환
- 유지보수:
  - 데이터 에셋/컴포넌트로 튜닝 가능한 구조로 정리

## 6. 제안 아키텍처(파일/클래스 단위)

### 6.1 이름 규칙

- 접두어 통일: `SB` 또는 `SimpleBattle` 중 하나로 통일(예: `ASBPlayerCharacter`)

### 6.2 추천 구성(최소 변경)

- Game:
  - `ASBGameMode`: 기본 폰/PC 설정, 테스트 스폰은 제거하고 Spawner로 이동
  - `ASBPlayerController`: 마우스/입력 모드

- Player:
  - `ASBPlayerCharacter`: 카메라 + 이동 + 커서 회전
  - `USBInputConfig`(DataAsset): IMC/IA 묶음 (현 `UInputDataAsset`를 유지/개명)

- Enemy/AI:
  - `ASBEnemyCharacter`: 외형 + 텔레그래프 + (선택) Health/Damage 인터페이스
  - `ASBEnemyAIController`: 현재 상태머신 유지, “경고”와 “실제 공격”을 분리 호출

- Components:
  - `USBAppearanceComponent`: 메시 구성은 유지 가능하되 머티리얼은 에셋 기반
  - `USBAttackTelegraphComponent`: 데칼 표시 + 와이프 애니메이션 (머티리얼 에셋 기반)

- Optional:
  - `ASBEnemySpawner`(Actor): 맵에 배치해서 스폰 파라미터(수/위치/간격) 관리

## 7. 단계별 작업 순서(삭제 후 재작성에 적합)

1. “실행 경로” 확정
   - `ASimpleGameMode`가 실제로 적용되는지 확인
   - 기본 폰이 BP인지 C++인지 확정(입력 에셋 주입 방식 결정)

2. 입력 파이프라인부터 고정
   - `USBInputConfig`(현 `UInputDataAsset`)를 기준으로:
     - IMC/IA가 Content에 존재하도록 만들고
     - 플레이어(또는 PC)에서 `AddMappingContext()`가 반드시 실행되도록 구성

3. 텔레그래프 컴포넌트 에셋 기반 전환
   - `USBAttackTelegraphComponent`:
     - `UMaterialInterface* TelegraphMaterial` (EditDefaultsOnly)
     - `UMaterialInstanceDynamic*` 생성 후 `RevealProgress`만 애니메이션
     - 데칼 컴포넌트를 DefaultSubobject로 소유

4. 외형 컴포넌트 에셋 기반 전환
   - `USBAppearanceComponent`:
     - 기본 primitive mesh 사용은 유지 가능(의존성 낮음)
     - 머티리얼은 Content 에셋(파라미터 `BaseColor`)을 사용

5. 적 AI: “경고”와 “공격 판정” 분리
   - 컨트롤러가:
     - 경고 시작 -> 일정 시간 후 실제 공격 이벤트 발생(대미지 처리) -> Idle
   - 공격 판정은:
     - 박스/캡슐 스윕 또는 데칼과 동일한 전방 사각형 형태로 계산(일관성)

6. 스폰/테스트 로직 정리
   - GameMode 하드코딩 스폰 제거
   - `ASBEnemySpawner`로 이동하거나 맵에 직접 배치

7. 설정 파일 정리
   - `DefaultGame.ini` 또는 프로젝트 세팅으로 기본 GameMode 명시(재현성)

## 8. 파일 매핑(현재 -> 제안)

- `SimpleBattle/Source/SimpleBattle/Public/Character/CharacterPlayer.*`
  - -> `.../Player/SBPlayerCharacter.*` (또는 이름만 정리)
- `.../Character/CharacterEnemy.*`
  - -> `.../Enemy/SBEnemyCharacter.*`
- `.../AI/EnemyAIController.*`
  - -> `.../AI/SBEnemyAIController.*`
- `.../Component/AttackWarningComponent.*`
  - -> `.../Component/SBAttackTelegraphComponent.*` (구현 방식 변경)
- `.../Component/StaticMeshAppearanceComponent.*`
  - -> `.../Component/SBAppearanceComponent.*` (머티리얼 생성 제거)
- `.../Game/SimpleGameMode.*`
  - -> `.../Game/SBGameMode.*`
- `.../Player/SimplePlayerController.*`
  - -> `.../Player/SBPlayerController.*`
- `.../Input/InputDataAsset.*`
  - -> 유지 또는 `SBInputConfig.*`

## 9. 수동 테스트 체크리스트

- 맵 진입 시:
  - 마우스 커서 표시 여부
  - 입력 바인딩 정상 여부(이동/점프/공격 로그)
- 플레이어:
  - 이동이 카메라 기준 축(+X/+Y)으로 자연스러운지
  - 커서 방향 회전이 지면 히트 기준으로 안정적인지
- 적:
  - Idle -> Move -> 경고 -> (공격) -> Idle 루프가 끊기지 않는지
  - 경고 데칼의 방향/길이/폭이 캐릭터 전방과 일치하는지
- 패키징 가정:
  - 머티리얼이 에셋 기반이면 WITH_EDITOR 의존 없이 동일하게 표시되는지

## 10. 안전한 삭제 범위(참고)

“프로젝트를 초기화”할 때 보통 아래는 삭제해도 재생성된다:

- `SimpleBattle/Binaries/`
- `SimpleBattle/Intermediate/`
- `SimpleBattle/Saved/`
- `SimpleBattle/DerivedDataCache/`

`Config/`, `Content/`, `Source/`, `*.uproject`는 유지가 기본이다.

