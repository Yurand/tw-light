/*
This file is part of "TW-Light"
					http://tw-light.appspot.com/
Copyright (C) 2001-2004  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef __MFLEET_H__
#define __MFLEET_H__

#include "melee.h"

class Fleet;
extern Fleet *reference_fleet;

#include <vector>				 //needed for STL list, the using namespace thingy allows STL classes

//TODO get rid of global variables used by this function; remove this function
void init_fleet();				 // inits reference_fleet [former shiptype array]

/**	\brief	Contains a list of ships. */
class Fleet
{

	protected:
		typedef unsigned int Index;
		typedef ShipType * MyFleetShipType;
		typedef std::vector<MyFleetShipType> MyFleetListType;

		static char * sortingMethodName[];
		static char * fleetCostName[];

	public:
		//the maxiumum length of the fleet title
		enum { MAX_TITLE_LENGTH = 80 };

		enum SortingMethod
		{
								 /**< combined Specied/ship name */
			SORTING_METHOD_NAME = 0,
			SORTING_METHOD_COST,
			SORTING_METHOD_NAME1,/**< Species name */
			SORTING_METHOD_NAME2,/**< Ship name (not including species) */
								 /**<  */
			SORTING_METHOD_CODERS,
								 /**< TW, or the group that made the ship */
			SORTING_METHOD_ORIGIN,
								 /**< Currently does the same thing as SORTING_METHOD_NAME_DESCENDING */
			SORTING_METHOD_DEFAULT = SORTING_METHOD_NAME,
			MAX_SORTING_METHODS = SORTING_METHOD_ORIGIN
		};

		/**
		 */
		static SortingMethod cycleSortingMethod(SortingMethod method) {
			method = (SortingMethod)((int)method+1);
			if (method > MAX_SORTING_METHODS) {
				method = (SortingMethod)0;
			}
			return method;
		}

		/**
		 */
		static const char * getSortingMethodName(SortingMethod method) {
			ASSERT( ! ( (method > MAX_SORTING_METHODS) || (method < 0)) );

			if ( (method > MAX_SORTING_METHODS) || (method < 0))
				return "";
			return sortingMethodName[method];
		}

		enum FleetCost
		{
			FLEET_COST_SMALL = 100,
			FLEET_COST_MEDIUM = 250,
			FLEET_COST_LARGE = 500,
			FLEET_COST_HUGE = 1000,
			FLEET_COST_MASSIVE = 10000
		};

		enum { FLEET_COST_DEFAULT = FLEET_COST_LARGE };
		enum { FIRST_FLEET_COST = FLEET_COST_SMALL };
		enum { MAX_FLEET_COSTS = FLEET_COST_MASSIVE };

		/** \brief cycles through each of the available preset maximum fleet sizes.
			\return the new fleet size */
		FleetCost cycleMaxFleetCost() {
			ASSERT (!( (maxFleetCost != FLEET_COST_SMALL) &&
				(maxFleetCost != FLEET_COST_MEDIUM) &&
				(maxFleetCost != FLEET_COST_LARGE) &&
				(maxFleetCost != FLEET_COST_HUGE) &&
				(maxFleetCost != FLEET_COST_MASSIVE) ));

			switch (maxFleetCost) {
				case FLEET_COST_SMALL:
					maxFleetCost = FLEET_COST_MEDIUM; break;
				case FLEET_COST_MEDIUM:
					maxFleetCost = FLEET_COST_LARGE; break;
				case FLEET_COST_LARGE:
					maxFleetCost = FLEET_COST_HUGE; break;
				case FLEET_COST_HUGE:
					maxFleetCost = FLEET_COST_MASSIVE; break;
				case FLEET_COST_MASSIVE:
					maxFleetCost = FLEET_COST_SMALL; break;
				default:
					ASSERT(0);
			}
			return maxFleetCost;
		}

	protected:
		/** \brief the maximum cost this fleet will allow */
		FleetCost maxFleetCost;

	public:

		static char * getFleetCostName(FleetCost size) {

			ASSERT (!( (size != FLEET_COST_SMALL) &&
				(size != FLEET_COST_MEDIUM) &&
				(size != FLEET_COST_LARGE) &&
				(size != FLEET_COST_HUGE) &&
				(size != FLEET_COST_MASSIVE) ));

			// this has got to be the worst solution ever :)
			int index = 0;
			switch (size) {
				case FLEET_COST_SMALL: index = 0; break;
				case FLEET_COST_MEDIUM: index = 1; break;
				case FLEET_COST_LARGE: index = 2; break;
				case FLEET_COST_HUGE: index = 3; break;
				case FLEET_COST_MASSIVE: index = 4; break;
				default:
					ASSERT(0);
			}
			return fleetCostName[index];
		}

		/** \brief Default constructor with zero ships */
		Fleet();

		/** \brief removes all ships and sets the cost to zero */
		void reset();

		/** \brief dumps out the fleet out to a buffer
			\param psize the size of the buffer
			\return a pointer to the buffer */
		void *serialize (int *psize);

		/** \brief reads in the fleet from a specified buffer
			\param data the buffer to read from
			\param psize the size of the buffer*/
		void deserialize(void *data, int psize);

		/** \brief adds a ship to this fleet
			\param type the type of ship to add
			\return the slot the ship type was added to
		*/
		int addShipType( ShipType * type );

		//depricated (no longer in use)
		//void select_slot(int slot, ShipType *type);

		/** \brief removes a given ship from the specified ship slot
			\param slot the slot the ship to be removed appears in */
		void clear_slot (int slot);

		/** \brief saves the fleet to a config file, given the config filename and section
			\param filename the filename to open and save to
			\param section the section within the specified config file to save to
			\TODO report an error code
		*/
		void save(const char *filename, const char *section);

		/** \brief loads the fleet from a config file (to this fleet), given the filename and section of a config file.
			\param filename the filename to save to
			\section the section within the config file to save to
		*/
		void load(const char *filename, const char *section);

		/** \brief sorts this fleet according to the given compare function.  May be called as Sort();
			\param method The method with which to sort.  Options are:
				-SORTING_METHOD_DEFAULT,
				-SORTING_METHOD_NAME_DESCENDING,
				-SORTING_METHOD_NAME_ASCENDING,
				-SORTING_METHOD_COST_DESCENDING,
				-SORTING_METHOD_COST_ASCENDING,
				-SORTING_METHOD_TWCOST_ASCENDING,
				-SORTING_METHOD_TWCOST_DESCENDING,
				-SORTING_METHOD_NAME1_ASCENDING,
				-SORTING_METHOD_NAME1_DESCENDING,
				-SORTING_METHOD_NAME2_ASCENDING,
				-SORTING_METHOD_NAME2_DESCENDING,
				-SORTING_METHOD_CODERS_ASCENDING,
				-SORTING_METHOD_CODERS_DESCENDING,
				-SORTING_METHOD_ORIGIN_ASCENDING,
				-SORTING_METHOD_ORIGIN_DESCENDING
			\param startIndex (default 0) the index of the first ship to sort
			\param endIndex (default -1) the index of the last ship in the fleet to sort.  -1 means the last ship.
		*/
		void Sort(SortingMethod method=(SortingMethod)SORTING_METHOD_DEFAULT, bool ascending=false, int startIndex=0, int endIndex=-1);

		/** \brief returns the number of ships in this fleet
			\return the number of ships in this fleet*/
		int inline getSize() { return ships.size(); }

		inline int getCost() { return cost; }
		inline int setCost(int newCost) { cost = newCost; return cost; }

		inline char * getTitle() { return title; }
		inline char * setTitle(char * newTitle) { strncpy(title, newTitle, MAX_TITLE_LENGTH); return title; }

		/** \brief adds the ships in the specified fleet to this one
			\param fleetToAdd the fleet with ships to add */
		void addFleet(Fleet * fleetToAdd);

		/** \brief Returns the ship in a particular slot.
			\param slot The slot of the ShipType to return.  Returns NULL if slot is not (0 <= slot <= getSize())
			\return The chosen ShipType, or NULL if slot was outside the selectable range of ShipTypes.*/
		ShipType * getShipType( int slot );

		/** \brief Returns the ship in a particular slot.
			\param offset The slot of the ShipType to return.  Returns NULL if slot is not (0 <= slot <= getSize())
			\return The chosen ShipType, or NULL if slot was outside the selectable range of ShipTypes.*/
		ShipType * operator[](int offset) {
			return getShipType( offset );
		}

		/** @brief returns the maximum number of ships that this fleet can hold */
		int getMaxNumberOfShips() { return MAX_FLEET_SIZE; }

		/** @brief returns the maximum sum of costs of each ship in the fleet */
		FleetCost getMaxCost() { return maxFleetCost; }

		/** @brief returns the next entry in the fleet with name starting with the given character.
			returns the next entry in the fleet with name starting with the given character.  The ship
			returned will be relative to currentShip.  It will be either the next ship past currentShip
			with name starting with c, or it will return currentShip if there's no such animal.  Seaching
			will start at currentShip, and if necessary, cylce past the end of the list back to the begining.
			@param currentShip the index of the ship to start searching from.
			@param c the character to search for
			@return the index of the ship whose name starts with c, and is next in the list,
			relative to currentShip.
		*/
		int getNextFleetEntryByCharacter(unsigned int currentShip, char c);

	protected:

		/** \brief the title of this fleet*/
		char title[MAX_TITLE_LENGTH];

		/** \brief the total cost of all ships in this fleet */
		int cost;

		/** \brief the list of ships*/
		MyFleetListType ships;

};
#endif							 // __MFLEET_H__
