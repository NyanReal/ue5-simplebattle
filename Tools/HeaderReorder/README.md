# UE5 Header Reorder Tool

Unreal Engine 5 프로젝트에서 `.generated.h` include가 잘못된 위치에 있을 때 자동으로 마지막으로 이동시키는 도구입니다.

## 요구사항

- .NET 10 SDK

## 빌드

```powershell
dotnet build
```

## 사용법

```powershell
# 기본 (최근 10분 수정 파일만)
dotnet run -- "D:\MyProject\MyGame"

# 전체 파일 처리
dotnet run -- "D:\MyProject\MyGame" --all

# 미리보기 (변경 없이 확인)
dotnet run -- "D:\MyProject\MyGame" --all --dry-run

# 상세 로그
dotnet run -- "D:\MyProject\MyGame" --all --dry-run --verbose
```

## 옵션

| 옵션 | 설명 | 기본값 |
|------|------|--------|
| `<path>` | UE5 프로젝트 경로 (.uproject 위치) | 필수 |
| `--all`, `-a` | 모든 파일 처리 (날짜 무시) | `false` |
| `--minutes`, `-m` | N분 이내 수정 파일만 처리 | `10` |
| `--dry-run`, `-d` | 변경 없이 미리보기 | `false` |
| `--verbose`, `-v` | 상세 출력 | `false` |

## 동작 방식

```cpp
// Before - .generated.h가 중간에 있음 (문제)
#include "MyClass.h"
#include "MyClass.generated.h"
#include "Engine.h"

// After - .generated.h가 마지막으로 이동
#include "MyClass.h"
#include "Engine.h"
#include "MyClass.generated.h"
```

## 스캔 경로

- `[Project]/Source/**/*.h`
- `[Project]/Plugins/*/Source/**/*.h`

## License

MIT
