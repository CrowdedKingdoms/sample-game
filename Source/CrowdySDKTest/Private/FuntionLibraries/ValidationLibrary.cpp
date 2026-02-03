// Fill out your copyright notice in the Description page of Project Settings.


#include "FuntionLibraries/ValidationLibrary.h"

bool UValidationLibrary::ValidateEmail(const FString Email)
{
	const FString Trimmed = Email.TrimStartAndEnd();

	const FRegexPattern Pattern(
		TEXT("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$")
	);

	FRegexMatcher Matcher(Pattern, Trimmed);
	return Matcher.FindNext();
}
