# 캐릭터 이미지 완성본 v1

- 제작일: 2026-10-25
- 대상: 레온 애쉬포드, 릴리안 블룸, 비비안 미스트우드, 엘레노어 나이트베일
- 제작 방식: 사용자 제공 Anima 원화 → 내장 image_gen으로 감정·동작·전투 원본 제작 → 사용자 승인에 따라 Python/Pillow로 분리·정렬·규격화
- 상태: 이미지 파일 편집 완료. C++ 게임 연결과 Windows 실행 확인은 별도다.

[전체 미리보기](../../art_sources/characters/preview.html) · [완성본 ZIP](../../artifacts/character_assets_v1.zip) · [출력 정보](../../src/RebornBoss/resources/character_assets.json) · [생성 원본·프롬프트 목록](../../art_sources/characters/production_v1.json)

## 완성 범위

| 분류 | 캐릭터별 구성 | 전체 수량 | 출력 규격 |
|---|---|---|---|
| 감정 초상화 | 기본·미소·분노·놀람·결의·피로/피격 6종 | 24장 | 320 × 480, RGBA PNG |
| 기본 초상화 별칭 | 기본 감정과 같은 파일 내용 | 4장 | 320 × 480, RGBA PNG |
| 전신 동작 | 대기·좌우 이동·시전·피격·등장·승리·패배 8종 | 32장 | 640 × 960, RGBA PNG |
| 전투 개별 프레임 | 9상태 × 4프레임 | 144장 | 레온 32 × 48, 보스 64 × 64 |
| 전투 시트 | 기본 1장·전체 1장·상태별 9장 | 44장 | 아래 파일 규격 참조 |
| 합계 | 네 캐릭터 | 248 PNG | 전부 RGBA |

전투의 등장·퇴장은 대기 프레임의 투명도를 단계적으로 바꾼 것이다. 나머지 7상태는 생성한 동작 원본에서 각 4프레임을 분리했다. 전신 이미지의 퇴장 연출은 등장 자세를 재사용할 수 있다.

## 게임용 저장 위치

`src/RebornBoss/resources/`를 기준으로 한다.

- `images/portraits/{캐릭터}_portrait_{감정}.png`: 감정별 초상화. 기본 파일명 `{캐릭터}_portrait.png`도 제공한다.
- `images/poses/{캐릭터}/{캐릭터}_{동작}.png`: 전신 동작 이미지.
- `images/characters/{캐릭터}_sheet.png`: 기존 리소스 계획과 호환되는 기본 시트.
- `images/characters/{캐릭터}_animations.png`: 전체 9상태 시트.
- `images/characters/{캐릭터}_animations.json`: 셀·기준점·행·시간·반복·개별 파일 정보.
- `images/characters/{캐릭터}/{동작}_sheet.png`: 동작 하나의 가로 4프레임 시트.
- `images/characters/{캐릭터}/{동작}_00.png`부터 `03.png`: 개별 프레임.
- `character_assets.json`: 네 캐릭터의 파일과 프레임 정보.
- `licenses/character_assets_v1.txt`: 원화·도구·후처리 제작 기록.

캐릭터 접두어는 `leon`, `lillian`, `vivian`, `eleanor`다. 파일명은 영문 소문자와 밑줄로 통일했다.

## 시트와 프레임

| 캐릭터 | 셀 크기 | 기본 시트 | 전체 시트 | 기준점 |
|---|---|---|---|---|
| 레온 | 32 × 48 | 4열 × 3행, 128 × 144 | 4열 × 9행, 128 × 432 | (16, 24) |
| 릴리안 | 64 × 64 | 4열 × 2행, 256 × 128 | 4열 × 9행, 256 × 576 | (32, 32) |
| 비비안 | 64 × 64 | 4열 × 2행, 256 × 128 | 4열 × 9행, 256 × 576 | (32, 32) |
| 엘레노어 | 64 × 64 | 4열 × 2행, 256 × 128 | 4열 × 9행, 256 × 576 | (32, 32) |

기본 시트는 레온이 대기·왼쪽 이동·오른쪽 이동 순서, 보스가 대기·시전 순서다. 전체 시트는 아래 행 번호를 사용한다. 프레임 순서는 왼쪽부터 0~3이며, 셀 간격은 0이다.

| 행 (0부터) | 동작 ID | 한 프레임 시간 | 반복 |
|---|---|---|---|
| 0 | `idle` | 120 ms | 예 |
| 1 | `move_left` | 100 ms | 예 |
| 2 | `move_right` | 100 ms | 예 |
| 3 | `cast` | 90 ms | 아니오 |
| 4 | `hit` | 80 ms | 아니오 |
| 5 | `victory` | 140 ms | 예 |
| 6 | `defeat` | 160 ms | 아니오 |
| 7 | `appear` | 120 ms | 아니오 |
| 8 | `exit` | 120 ms | 아니오 |

반복하지 않는 동작은 마지막 프레임을 유지하도록 기록했다. 게임에서 다음 상태로 바꾸는 시점은 게임 로직이 결정한다. 등장 첫 프레임과 퇴장 마지막 프레임은 완전히 투명한 정상 프레임이다.

전체 시트의 잘라 그릴 영역은 `x = 프레임 번호 × 셀 가로`, `y = 행 번호 × 셀 세로`다. 화면 좌표에서 기준점을 빼서 배치한다. 충돌 반경은 이미지 크기와 별도로 정한다. 픽셀 이미지는 최근접 샘플링과 정수 배율 표시를 사용한다.

## 원본과 재생성

- 생성 원본: `art_sources/characters/{캐릭터}/expressions_source_v1.png`, `poses_source_v1.png`, `combat_source_v1.png`
- 생성 요청문과 도구 기록: 같은 폴더의 `*.prompt.txt`, `*.generation.json`
- 원본의 분할 구성: `art_sources/characters/production_v1.json`
- 출력·검증: `tools/build_character_assets.py`
- 경계·정렬·투명 프레임 등의 회귀 검사: `tools/test_character_assets.py`
- 출력 해시와 규격 확인 결과: `art_sources/characters/validation_v1.json`

Python 3.10 이상과 Pillow 10.1 이상이 있는 환경에서 프로젝트 루트에서 실행한다.

```sh
python tools/build_character_assets.py
python tools/build_character_assets.py --check
python -m unittest discover -s tools -p test_character_assets.py
```

재생성은 보관한 원본 시트를 사용하므로 이미지 생성 서비스를 다시 호출하지 않는다. 전투 이미지는 공유 팔레트 48색 이하, 불투명/투명 픽셀 경계와 셀 내부 2px 여백으로 정리한다. 등장·퇴장에만 단계적 알파를 사용한다. 초상화는 상반신 구도로 크롭하고, 전신 동작은 별도의 파일로 제공한다.

## 확인한 범위

- 네 캐릭터의 감정 6종과 전신 동작 8종을 분리하고, 얼굴·의상·종족 특징을 확인했다.
- 248 PNG의 RGBA 모드, 크기, 파일 존재, 기본 초상화 별칭을 검사했다.
- 전투 개별 프레임·상태별 시트·전체 시트의 픽셀 일치, 2px 투명 여백, 알파와 색상 수를 검사했다.
- 공격·피격·패배에 서로 다른 동작 프레임이 있는지 검사했다.
- 브라우저에서 감정 전환, 전투 동작 선택과 재생, 숲·호수·신전 배경 위 표시를 확인했다.
- 후처리 스크립트의 회귀 검사 7개를 통과했다.
- ZIP을 새 폴더에 풀어 보관 원본에서 재생성한 PNG 248개가 전달 PNG와 SHA-256까지 모두 일치했다. 결과는 `art_sources/characters/rebuild_check_v1.json`에 보관했다.

현재 C++ 소스에는 이미지 로딩·상태별 애니메이션 연결이 아직 구현되지 않았다. 이미지 완성 상태를 게임 적용 완료로 표시하지 않았다. Visual Studio 컴파일과 Windows의 glc2d 실제 표시·재생은 사용자가 확인한다.
