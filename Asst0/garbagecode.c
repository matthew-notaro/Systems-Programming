printf("partitioning\n");
//Set pivot to be data of first Node
void* pivot = left->data;

//Set partition node to the first non-pivot Node
Node* partNode = left;

Node* i;

//Traverse list
for(i = partNode; i != right; i = i->next)
{
  printf("i data: %s, piv data: %s,", i->data, pivot);
  printf(" comparator %d\n", comparator(i->data, pivot));

  //If data of current Node i is less than pivot
  if(comparator(i->data, pivot) <= 0) //if current comes before
  {
    partNode = partNode->next;
  }
}
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
