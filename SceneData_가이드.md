# SceneData 가이드

이 문서는 현재 `KirbyEngine`의 SceneData 저장/로드 방식과,
무엇이 저장되고 무엇이 저장되지 않는지를 정리한 문서입니다.

## 1. SceneData란?

SceneData는 scene 상태를 JSON으로 저장한 파일입니다.

보통 위치는 아래 폴더입니다.

```text
Solution_Kirby/SceneData
```

실행 중에는 `SceneDataManager`가 이 경로를 찾고,
저장과 로드를 담당합니다.

## 2. 현재 SceneData 버전

현재 최신 SceneData 버전은 `9`입니다.

최근 변화:

- v8
  - component별 `componentId` 추가
- v9
  - registry 기반 `referenceFields` 추가

## 3. 최상위 구조

대략 이런 형태입니다.

```json
{
  "version": 9,
  "sceneName": "Sample",
  "timeScale": 1.0,
  "camera": { ... },
  "objects": [ ... ]
}
```

## 4. object에 저장되는 것

object에는 보통 아래 정보가 들어갑니다.

- `id`
- `parentId`
- `name`
- `tag`
- `active`
- `transform`
- `components`

즉,
계층 구조와 transform, 활성 상태, component 목록까지 같이 저장됩니다.

## 5. component에 저장되는 것

component entry는 대략 아래 구조입니다.

```json
{
  "type": "SampleSpinComponent",
  "componentId": 12,
  "data": {
    ...
  }
}
```

### 의미

- `type`
  - 어떤 component인지 알려 주는 문자열
- `componentId`
  - component를 다시 찾기 위한 고유 id
- `data`
  - 각 component가 직접 저장하는 값

## 6. referenceFields

registry 기반 참조 필드는 `data` 아래 `referenceFields`에 저장됩니다.

예:

```json
"referenceFields": {
  "Registry Target Object": {
    "kind": "GameObject",
    "gameObjectId": 23
  },
  "Any Component": {
    "kind": "Component",
    "componentId": 41,
    "expectedType": ""
  },
  "Rigidbody2D Only": {
    "kind": "Component",
    "componentId": 12,
    "expectedType": "Rigidbody2D"
  }
}
```

### 저장 규칙

- `GameObject` 참조는 `gameObjectId` 저장
- `Component` 참조는 `componentId` 저장
- 비어 있으면 `-1`

## 7. 로드할 때의 흐름

로드는 한 번에 포인터를 바로 연결하지 않습니다.

대략 순서는 이렇습니다.

1. 모든 `GameObject` 생성
2. 모든 `Component` 생성
3. 각 component 기본 데이터 `Deserialize()`
4. registry 기반 reference id 임시 보관
5. 모든 object/component가 다 만들어진 뒤 resolve pass 실행
6. `gameObjectId`, `componentId`를 실제 포인터로 연결

이렇게 하는 이유는,
아직 안 만들어진 대상을 먼저 가리키려고 하면 연결할 수 없기 때문입니다.

## 8. 저장되는 것 / 저장되지 않는 것

### 저장되는 것

- scene 이름
- timeScale
- camera 정보
- GameObject id / parentId / name / tag / active / transform
- component type
- componentId
- component별 data
- registry 기반 referenceFields
- 일부 action key 같은 문자열 기반 연결 정보

### 저장되지 않는 것

- raw pointer
- DirectX texture pointer
- Box2D body pointer
- lambda / callback 본체
- editor 창 상태

즉,
다시 만들어 낼 수 없는 런타임 포인터는 저장하지 않는 쪽이 기본입니다.

## 9. 구버전 호환

현재는 구버전 SceneData도 가능한 한 계속 읽을 수 있게 유지하고 있습니다.

예:

- `componentId`가 없는 파일도 로드 가능
- `referenceFields`가 없는 파일도 로드 가능
- 이 경우 없는 값은 새로 만들거나 `nullptr`로 둠

즉,
최신 포맷만 강제하는 방식보다는
이전 테스트 씬과 샘플 씬도 같이 살리는 방향입니다.

## 10. component를 만들 때 주의할 점

새 component를 만들 때 SceneData와 연결하려면 보통 아래를 생각하면 됩니다.

- `GetSerializableType()` 구현
- `Serialize()` / `Deserialize()` 구현
- 참조 필드가 있으면 `RegisterReferenceFields()` 등록
- 수동 참조를 쓰면 `ResolveReferences()`도 직접 처리

## 11. 현재 확인용 예제

SceneData와 registry 기반 reference field를 같이 보기 좋은 예제는:

- `ReferenceFieldRegistryTestComponent`

입니다.

이 컴포넌트는
manual reference field와 registry 기반 reference field를 같이 가지고 있어서,
저장/로드 확인용으로 보기 좋습니다.

## 12. 한 줄 정리

현재 SceneData는
"object/component 기본 데이터 + componentId + registry 기반 참조 정보까지 JSON으로 저장하는 구조"
로 이해하면 됩니다.