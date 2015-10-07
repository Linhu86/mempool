#include "MemoryPoolManager.hpp"
#include "StandardMemoryPool.hpp"
#include "tinyxml.h"
#include <assert.h>

const char* const MemoryPoolManager::c_poolsFileXML = "pools.xml";

/**
*	\brief		Construct the manager reading from the provided xml files all the pools
*/
MemoryPoolManager::MemoryPoolManager(const char* const poolsFileXML)
{
  TiXmlDocument doc( poolsFileXML );
  if(doc.LoadFile())
  {
    TiXmlNode* root = doc.FirstChild("memorypools");
    assert(root);
    if(root)
    {
      TiXmlNode* pool = root->FirstChildElement();
      //Create Pools
      while(pool)
      {
        if(pool->Type() == TiXmlNode::TINYXML_ELEMENT && isAValidPoolType(pool->Value()))
        {
          TiXmlElement* element = pool->ToElement();
          if(element)
          {
            TiXmlAttribute* attribute = element->FirstAttribute();

            char name[128] = {'\0'};
            uint32 size = 1024;
            int boundCheck = false;

            while(attribute)
            {
              if(strcmp(attribute->Name(), "name") == 0)
              {
                strncpy(name, attribute->Value(), 128);
              }
              else if(strcmp(attribute->Name(), "size") == 0)
              { 
                size = attribute->IntValue();
              }
              else if(strcmp(attribute->Name(), "boundsCheck") == 0)
              {
                boundCheck = attribute->IntValue() == 1 ? true : false;
              }
              attribute = attribute->Next();
            }

            MemoryPool* memPool = NULL;

            if(strcmp(pool->Value(), "pool") == 0)
            {
              memPool = new StandardMemoryPool(size, boundCheck);
            }

            if(memPool)
            {
              m_pool.insert(std::pair<std::string, MemoryPool*>(name, memPool));
            }
          }
        }

        pool = pool->NextSibling();
      }
    }
  }
  else
  {
    printf("Memory pool set XML file not found");
  }
}

/**
*	\brief		Check if the pool type in string form is a valid recognized pool type.
*/
int MemoryPoolManager::isAValidPoolType(const char* poolType) const
{
  if(strcmp(poolType, "pool") == 0) 
  {
    return true;
  }
  
  return false;
}

