#include "JSONObj.hpp"

JSONObj::JSONObj() { umSingleObjs.clear(); }

JSONObj::~JSONObj() {}

HRESULT JSONObj::addSingleObj(std::wstring wsKey, std::wstring wsValue) {
  if (umSingleObjs.find(wsKey) != umSingleObjs.end()) {
    return E_FAIL;
  }
  umSingleObjs.insert({wsKey, wsValue});
  return S_OK;
}

std::wstring JSONObj::toString() {
  std::wstring wsRes = L"{\n";
  for (auto &p : umSingleObjs) {
    wsRes += std::format(L"\t\"{}\": \"{}\",\n", p.first, p.second);
  }
  wsRes += L"}\n";
  return wsRes;
}