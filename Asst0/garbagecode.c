//partNode = (partNode == NULL) ? left : partNode->next;

if(partNode == NULL)
{
  partNode = left;
}
else if(partNode->next != NULL)
{
  partNode = partNode->next;
}

printf("partNode: %s, right: %s\n", (void*)partNode->data, (void*)right->data);

// void* temp = partNode->data;
// partNode->data = i->data;
// i->data = temp;

//swapNodeData(partNode, right);

return partNode;
