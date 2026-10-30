"""Run the real trick handler against lightweight stand-ins (not an Unreal build)."""
import pathlib
import subprocess
import tempfile

root = pathlib.Path(__file__).resolve().parents[2]
source = (root / "SSXPure/Source/SSXPure/SSXPureSnowboarder.cpp").read_text()
start = source.index("void ASSXPureSnowboarder::UpdateTricks(float DT)")
end = source.index("\nvoid ASSXPureSnowboarder::UpdateCamera", start)
handler = source[start:end]

stubs = r'''
#include <cassert>
#include <cmath>
#include <cstdint>
#include <string>
using int32 = int32_t;
#define TEXT(x) x
struct FVector { float X=0, Y=0, Z=0; FVector operator-(FVector v) const { return {X-v.X,Y-v.Y,Z-v.Z}; } };
struct FRotator { FVector Euler() const { return {}; } };
struct Mesh { FRotator GetComponentRotation() const { return {}; } };
struct FString { std::string value; FString& operator=(const char* s) { value=s; return *this; }
  template<class... Args> static FString Printf(const char*, Args...) { return {}; } };
struct FMath { static float Abs(float v) { return std::fabs(v); }
  static int32 FloorToInt(float v) { return static_cast<int32>(std::floor(v)); }
  static float Min(float a, float b) { return a<b?a:b; } };
struct ASSXPureSnowboarder {
  Mesh* BoardMesh; FVector LastRot; bool bIsAirborne=false;
  float TotalSpin=0,TotalFlip=0,AirTime=0,ComboMult=1,TrickDisplayTimer=0;
  int32 TotalScore=0; FString CurrentTrickName;
  void UpdateTricks(float DT);
};
'''
checks = r'''
int main() {
  Mesh mesh; ASSXPureSnowboarder rider{}; rider.BoardMesh=&mesh;
  rider.bIsAirborne=true; rider.AirTime=.2f; rider.TotalSpin=270;
  rider.UpdateTricks(.016f);
  rider.bIsAirborne=false; rider.UpdateTricks(.016f);
  assert(rider.TotalSpin==0 && rider.TotalFlip==0 && rider.AirTime==0);
  assert(rider.TotalScore==0 && rider.ComboMult==1);
  rider.bIsAirborne=true; rider.AirTime=1; rider.TotalSpin=360;
  rider.UpdateTricks(.016f);
  rider.bIsAirborne=false; rider.UpdateTricks(.016f);
  assert(rider.TotalScore==160 && rider.ComboMult==1.5f);
  rider.UpdateTricks(.016f); assert(rider.TotalScore==160);
  rider.bIsAirborne=true; rider.AirTime=.4f; rider.UpdateTricks(.016f);
  rider.bIsAirborne=false; rider.UpdateTricks(.016f);
  assert(rider.ComboMult==1 && rider.AirTime==0);
}
'''
with tempfile.TemporaryDirectory(dir=root) as temp:
    cpp = pathlib.Path(temp) / "trick_test.cpp"
    binary = pathlib.Path(temp) / "trick_test"
    cpp.write_text(stubs + handler + checks)
    subprocess.run(["c++", "-std=c++17", "-Wall", "-Wextra", "-Wno-unused-parameter", "-Werror", str(cpp), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
print("Trick landing regression passed (isolated handler; no Unreal Engine).")
