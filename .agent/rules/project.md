---
trigger: always_on
---

대답은 항상 한국어
계획, md 문서 작성도 한국어
코드내 주석은 영어

언리얼 엔진 빌드는 직접하지 않고 유저에게 요청한다

## C++ 코딩 규칙 (Unreal Engine)

### 헤더 파일 include 순서
- **`.generated.h` 파일은 반드시 마지막 `#include`여야 함**
- 올바른 순서:
  1. `#include "CoreMinimal.h"` (첫 번째)
  2. 기타 Engine/Module 헤더들
  3. 동일 모듈 내 헤더들
  4. `#include "ClassName.generated.h"` (마지막)

```cpp
// 올바른 예시
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryTypes.h"
#include "InventoryComponent.generated.h"  // 반드시 마지막!
```

> [!IMPORTANT]
> **AI가 헤더 파일(`.h`)을 수정한 경우, 반드시 HeaderReorder 도구를 실행해야 합니다:**
> 
> ```powershell
> cd d:\github\miyakov\Tools\HeaderReorder; dotnet run -- "d:\github\miyakov\Miyakov" --all
> ```
> 
> 이 도구는 자동으로 `.generated.h`가 마지막 include가 되도록 재정렬합니다.

> [!TIP]
> **수동 실행이 필요한 경우 (헤더 순서 에러 발생 시):**
> 
> ```powershell
> cd d:\github\miyakov\Tools\HeaderReorder
> dotnet run -- "d:\github\miyakov\Miyakov" --all
> ```

---

> [!IMPORTANT]