#include "UI/Marker/GS_MarkerSelectionWidget.h"

void UGS_MarkerSelectionWidget::SelectMarker(EMarkerType MarkerType)
{
	OnMarkerSelected.Broadcast(MarkerType);
	RemoveFromParent();
}

