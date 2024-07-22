#include "f4se/GameReferences.h"
#include "f4se/GameExtraData.h"
#include "f4se/GameRTTI.h"

// 
RelocPtr <PlayerCharacter*> g_player(0x0303ACA0);

RelocAddr <_HasDetectionLOS> HasDetectionLOS(0x0105A6E0);

RelocAddr <_GetLinkedRef_Native> GetLinkedRef_Native(0x00510600);

RelocAddr <_SetLinkedRef_Native> SetLinkedRef_Native(0x00510620);

RelocAddr <_MoveRefrToPosition> MoveRefrToPosition(0x010FB0F0);

bool Actor::GetEquippedExtraData(UInt32 slotIndex, ExtraDataList ** extraData)
{
	// Invalid slot id
	if (slotIndex >= ActorEquipData::kMaxSlots)
		return false;

	// This should be possible but check anyway
	if (!equipData)
		return false;

	// Make sure there is an item in this slot
	auto item = equipData->slots[slotIndex].item;
	if (!item)
		return false;

	if (!inventoryList)
		return false;

	// Find the equipped form from the inventory
	for (UInt32 i = 0; i < inventoryList->items.count; i++)
	{
		BGSInventoryItem inventoryItem;
		inventoryList->items.GetNthItem(i, inventoryItem);
		if (inventoryItem.form != item || !inventoryItem.stack)
			continue;

		// Search stacks for the equipped stack
		bool ret = inventoryItem.stack->Visit([&](BGSInventoryItem::Stack * stack)
		{
			if (stack->flags & BGSInventoryItem::Stack::kFlagEquipped)
			{
				ExtraDataList * stackDataList = stack->extraData;
				if (stackDataList) {
					(*extraData) = stackDataList;
				}

				return false;
			}

			return true;
		});
		if (!ret) // Stack found
			break;
	}

	return (*extraData) != nullptr;
}
