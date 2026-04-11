#include "System/HFunctionLibrary.h"

FString UHFunctionLibrary::MakeGemTagString(FName InGemID)
{
	if (InGemID.IsNone()) return TEXT("Data.GemID");
	return FString::Printf(TEXT("Data.GemID.%s"), *InGemID.ToString());
}
