/*
This file is part of "TW-Light"
					http://tw-light.appspot.com/
Copyright (C) 2001-2004  TimeWarp development team
Copyright (C) 2008  Yura Siamashka <yurand2@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

/*! \mainpage TW-Light
 *
 *  \section overview_sec Overview: What is TW-Light?
 *
 *  TW-Light is lightweighted version of Star Control: TimeWarp. Currently it includes only a combat
 *  portion with the best TW ships but without network support. There are some plans to expand it to
 *  include an epic adventure.
 *
 *  \section getting_started_sec Getting Started
 *
 * - Melee:  Starts a battle.  In battle the following keys do stuff:
 *       - F1 brings up the help screen (this file, at the moment)
 *       - F2 brings up the options menu
 *       - F3 switches camera focus
 *       - F4 is fast-forward (speeds up in-game time greatly)
 *       - F5 displays fleet status
 *       - F7 changes game tic rate (physics quality)
 *       - F8 changes camera mode
 *       - F9 creates planets (silly "feature")
 *       - F10 quits (so does ESCAPE)
 *       - F11 saves screenshots
 *       - F12 displays framerates / performance data
 *       - - zooms out on some viewing modes.
 *       - + (or =) zooms in on some viewing modes.
 *       - 0 and 9 also effect the camera in some viewing modes.
 *       - ctrl+T = toggle team indicators on/off
 *       - ctrl+H = toggle healthbar indicators on/off
 *       - Also, if some controllers are set to keyboard, customizable
 *	 - buttons may cause ship actions.  Be default these are:
 *       - Config 0:
 *               - left:   Keypad 4
 *               - thrust: Keypad 8
 *               - right:  Keypad 6
 *               - fire:   Quote (")
 *               -  special:Semicolon (;)
 *               - next:   Closebrace (])
 *               - prev:   Openbrace ([)
 *               - near:   P
 *       - Config 1:
 *               - left:   A
 *               - thrust: S
 *               - right:  D
 *               - fire:   B
 *               - special:V
 *               - next:   F
 *               - prev:   G
 *               - near:   H
 * - Extended Menu:
 *	- Play Game:
 *              - Same as "Melee" now
 *      - Key Tester:
 *              - Use this utility to check keys combination for conflicts.
 *	- Ship Info:
 *		- Here you can get detail information about every ship in the game.
 * 	- Diagnostic:
 *              - Show comlile options and version information.
 *	- Main Menu:
 *              - This returns to the main menu.
 *
 * - Teams:
 *      - Select Controller:
 *             - Select the player (i.e. "Player 1" or "Player 2")
 *              you wish to alter from the list on the left, and
 *              the controller (i.e. "Keyboard", "MoronBot") you
 *              want to control that player from the list on the
 *             left.  Then either click on the "Select
 *             Controller" button or double click on the
 *             controller name.
 *	- Change Team #:
 *	       - This switches the player allys.
 *	      Note that team 0 mean no allys.
 *      - Change Config #:
 *             -   This switches the configuration used by the
 *              currently highlighted controller.
 *      - Edit Configuration:
 *             -   This is used to setup up keys when the currently
 *              highlighted player is using the keyboard.  It
 *              may eventually allow the configuration of AIs
 *              and calibration of joysticks.
 *      - Edit Fleet:
 *             -   This button brings up the fleet selection menu for
 *              the currently highlighted player.
 *      - Main Menu:
 *             -   This returns to the main menu.
 * - Options:
 *      -  Most things on this menu are self-explanatory, but a few
 *         need special mention.
 *
 *      - Gamma correction:
 *             -  This makes things brighter during combat.  This
 *                 should not be changed from the middle of combat,
 *                 or colors could get weird.
 *      - Antialiasing:
 *             -  If this box is checked then TimeWarp will use
 *                higher quality graphics, but run slower.
 *      - Color Depth:
 *             -  You cannot change this from the middle of combat.
 *      -  Camera Mode:
 *             -  This allows you to control what is shown on your
 *                 screen in combat.
 *
 *             - "Enemy_Discrete"
 *                 is just like Star Control on the PC, where
 *                 the camera moved to keep both you and your
 *                 enemy onscreen, and zoomed in by factors of
 *                 2.
 *             -  "Enemy"
 *                 is like Star Control on the Genesis or 3DO,
 *                 where the camera zoomed in smoothly.
 *             -  "Hero"
 *                 makes the camera stay focused on Player 1,
 *                 and zoom in/out when + and - are pressed.
 *             -  "Everything"
 *                 makes the camera stay on the planet and zoom
 *                 way out so that everything is visible.  This
 *                 mode is slightly buggy (visual artifacts).
 *
 * \section development Development
 *
 * Following documents describes some of the issues concerning contributing
 * to Timewarp
 * 		- \ref coding_page
 * 		- \ref howto_document
 *
 * Currently we are working on the following tasks:
 *
 *   - Introduce the most interesting and cool TimeWarp ships
 *   - Simplify and fix melee engine
 *   - Add TimeWarp Markup Language (TML) support
 *   - Write plot
 *   - Implement plot with TML
 *
 * \subsection intoro Introduce the most interesting and cool TimeWarp ships
 *
 * Currently TW-Light includes the following ships:
 *
 *   - Alary Battlecruiser
 *   - Bipole Katamaran
 *   - The Chorali Extractor
 *   - Confed Cargotran
 *   - Confederation Hornet
 *   - Djinni Lancer
 *   - Drax Griffon
 *   - Earthling Crusader MK2
 *   - Earthling Crusader MK3
 *   - Zekfahan Shocker
 *   - Garash Tyrant
 *   - Hydrovar
 *   - Ilwrath Spider
 *   - Kahr Boomerang
 *   - Re-Koj Assassin
 *   - Rogue Squadron
 *   - Tau Archon
 *   - Tau Dagger
 *   - Tau Slider
 *   - Tau Torrent
 *   - Zekfahan Shocker
 *
 * \subsection simplyfy_subsec Simplify and fix melee engine
 *
 *  We are working on it.
 *
 * \subsection TML_subsec Add TimeWarp Markup Language (TML) support
 *
 *  No progress.
 *
 * \subsection plot_subsec Write plot
 *
 *  No progress.
 *
 * \subsection plot_impl_subsec  Implement plot with TML
 *
 *  No progress.
 *
 * \section licene_sec License
 *
 *  Copyright (C) 2004-2008 <a href=mailto:yurand2@gmail.com>Yura Siamashka</a>
 *  Copyright (C) 2001-2004 TimeWarp development team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 * \section credits_sec Credits
 *
 * TW-Light is derivative work from <a href="timewarp.sourceforge.net">Star Control: TimeWarp</a>

 */

/*! \page coding_page A guide to a few of the wierder issues with programming for TW-Light.
 *
 *
 * \section separation_phy_rend_sec Separation of Physics & Rendering
 *
 * Rendering functions (things like SpaceObject::animate()) are NOT allowed
 * to do any physics.  Not even a little bit.  This means that they are not
 * allowed to change any values in any SpaceObjects.
 *
 * \section random_numb_generator Random Number Generation
 *
 * On the same topic, there are two random number generators used by TW-Light;
 * the libc one, rand(), and a custom one, random().  Use random() when doing
 * physics, rand() when doing other things.  The reason for this is that
 * random() is synchronized in network games; rand() is not.  Using rand()
 * for something that's synchronized will desynchronize the game; and using
 * random() for something that's not synchronized can desynchronize the game.
 * Note: there is now a method of Control called rand(), which calls the
 * normal ::rand() or ::random() as appropriate, so you should use that one
 * for AI logic.
 *
 * If you don't understand this, try this rule of thumb: always use random()
 * unless you're in a method named animate or in a Control.
 * Additional Notes: random() returns a random number over the range of all
 * non-negative integer values.  random(7) is equivalent to (random()%7) in
 * that it returns an integer from 0 to 6 inclusive.  random(7) is faster than
 * (random()%7).  However, there are some subtleties to passing arguments to
 * random().  random(7.0) returns a real number X such that 0 <= X < 7.
 * random(-7.0) returns a real number X such that 0 >= X > -7.  random(-7)
 * will return gobbly-gook: random() should not be called with a negative
 * integer.  random(-3, 5) returns a real number from X such that -3 <= X < 5.
 * random(Vector2(3,1)) returns a random vector V such that 0 <= V.x < 3 and
 * 0 <= V.y < 1.  You don't get all these convienient helpers when calling
 * ::rand() or Control::rand().
 *
 * Final Note: The implementation of tw_random() is available as a class.
 * If you need additional streams of random numbers or some other strangeness,
 * you can create your own instance of it.  Starfields are handled that way.
 *
 * \section trivial_conventions_sec Trivial Conventions
 *
 * It would be nice if the following conventions were followed in TW-Light code:
 * -#  The proper indentation amount is 1 tab.  If you don't like that (or if
 * your editor produces spaces when you hit tab), then try 2 spaces instead.
 * If you don't like that either, AT LEAST BE INTERNALLY CONSISTENT IN YOUR
 * INDENTATION POLICY!  YES, THIS MEANS YOU!!!
 * -#  In the same vein, do not use an editor that transparently replaces N
 * spaces with the "equivalent" tabs.  Or, if you must do so, make sure that
 * your indentation amount exactly matches the tab size.  Otherwise your
 * indentation will apear totally fucked up on other editors with different tab
 * sizes.  And don't use an editor that replaces tabs with spaces either.
 * Don't use strange characters in file names.  To be specific, the 26
 * english characters and underscores and up to one period per file name
 * are okay.  I guess numbers would be ok too.  Use lower case letters,
 * never upper case.
 * -# Don't name two source files exactly the same thing, even if they're in
 * different directories. The same rule applies to headers.
 * -# All source files should have extensions of either .cpp or .h .  No .c
 * or .cc files.  Exception: .c files are permitted in the util directory.
 *
 * \section collisions_sec Collisions
 *
 * If you want to know whether or not you can collide with something, call
 * int canCollide(SpaceLocation *other) and other->canCollide(this).  Things
 * can only collide if BOTH return non-zero.
 * If you want to change what you can collide with you can either override
 * canCollide or you can modify the member variables collide_flag_ally and
 * collide_flag_anyone.  If you override canCollide, never return true, always
 * return either false or a base class' canCollide(other).
 *
 * \section messages_sec Messages
 *
 * There is a global variable called message.  This allows you to display
 * messages at the top of the screen during gameplay.  Use it like this:
 * message.out ("Hello World!");
 * // prints "Hello World!" for 1 second in pallete color 15 (white)
 * message.out ("Hello World!", 4500);
 * // prints "Hello World!" for 4500 milliseconds in pallete color 15 (white)
 * message.out ("Hello World", 4500, 9);
 * // prints "Hello World!" for 4500 milliseconds in pallete color 9 (light blue).
 * message.print (4500, 9, 0, "Hello World (number %d, string %s)", 99, "cheese");
 * // prints "Hello World (number 99, string cheese)" for 4500 milliseconds in pallete color 9
 * This stuff behaves somewhat differently if you use it before a game is
 * started or while the game is paused.
 * Oh, and you have to include mview.h (or melee/mview.h)
 * to do this.
 *
 * \section report_error_sec Reporting Errors
 *
 * If you wish to report an error condition, you can say something like:
 * tw_error("Oh no! an error occured!");
 * //simple error message
 * tw_error("This error was brought to you by the letter %c and the number %d", 'g', 3);
 * //error message using more complex printf-style stuff
 * When you report an error in this manner, a box will pop up and tell
 * the user your message, and the source file and line number from which
 * tw_error() was called.  The user will be presented with a number of buttons,
 * like "Abort", "Retry", and "Debug".  The "Abort" button will cause the game
 * to abort and dump the user back at the main menu.  The "Retry" button will
 * cause the tw_error() call to return as if nothing happened.  The "Debug"
 * button will attempt to stop the program in a debugger friendly manner so
 * that you can see the exact circumstances in which the error occured if you
 * have a good debugger installed.
 * Update: there is now also an "Ignore" button, which causes future error
 * messages to be suppressed.  Currently it causes all future error messages
 * to be suppressed, but in the future it may only cause error messages
 * generated by the same tw_error() call to be suppressed.
 *
 * TW-Light also keeps a log of certain events for debugging reason.  It's not
 * recommended, but you can add your own events to that log.  Simply call
 * debug_log() with printf-style parameters and your stuff will be added.
 * The log is written to tw_sys.log.  The log is used  sparingly in TW,
 * primarily for initialization of IO stuff.
 *
 * \section query_sec Query
 *
 * Query is an interface for finding items based upon position & layer.  You
 * call Query::begin( item, layers, radius) and it will search for items that
 * match these criteria:
 * -# within radius of item (measured from center of gravity for objects, from
 * beginning for lines)
 * -# within a layer specified by layers (using the same format as
 * collide_flag_anyone and collide_flag_ally)
 * -# not item itself
 * At that point, Query::current will contain the first item found, or NULL
 * if none were found.  If you are sure that Query returned only objects, you
 * can refer to Query::currento instead, or if you're sure it returned only
 * lines, refer to Query::currentl.  Call Query::next() when you want to find
 * the next item.  Query searches intelligently by location (i.e. it is fast
 * if there are few objects in the region you are searching).
 * Note: there are now some additional types of queries available that don't
 * require that the search center around an item...
 * Update: now a Query2 is also available, which uses attributes instead of
 * layers.  Eventually Query will be phased out in favor of Query2, at which
 * time Query2 will be renamed to Query.  I'll document how to use Query2
 * later...
 *
 * \section control_sec Controls:
 *
 * There is a class called Control, which is commented in melee/mcontrol.h.
 * This class should be used by all things that control ships: keyboards,
 * joysticks, AIs, etc..  It is intended to allow AIs to monitor any important
 * events like which ships are added to the game.
 *
 * \section layers_sec Layers
 *
 * (note: layers will be removed from the code eventually, and replaced by
 * attributes...)
 * Each item (SpaceObject or SpaceLine) in TimeWarp is in a layer.  The layer
 * also helps determine what things an object can collide with.  Some of the
 * layers are:
 * - LAYER_SHOTS       (for non-laser weapons, do NOT assume type Shot))
 * - LAYER_LINES       (for lasers/lines.  you may assume type SpaceLine)
 * - LAYER_SHIPS       (for ships, do NOT assume type Ship)
 * - LAYER_EXPLOSIONS  (for explosions)
 * - LAYER_CBODIES     (for cellestial bodies, i.e. asteroids and planets)
 * - LAYER_HOTSPOTS    (for hotspots)
 *
 * There are a number of things that deal with sets of layers.  For instance,
 * an item L can only collide with allies that are in layers that are in the
 * the set L.collide_flag_ally.  There is a similiar set, collide_flag_anyone
 * used for non-allies.  (Ally in this sense means things associated with the
 * same Control).  Additionally, Querys (described above) search for items in
 * sets of layers.  These are ways to describe the sets of layers:
 * - ALL_LAYERS        every normal layer
 * - OBJECT_LAYERS     every layer that contains only SpaceObjects
 * - LINE_LAYERS       every layer that contains only SpaceLines
 * - bit(L)            only the layer L
 * - 0                 no layers
 *
 * Additionally, given two sets of layers, S1 and S2,
 * - int S3 = S1 | S2      S3 is all layers that are in either S1 or S2 (or both)
 * - int S3 = S1 & S2      S3 is those layers in S1 that are also in S2
 * - int S3 = S1 &~S2      S3 is those layers in S1 that are not in S2
 *
 * Sorry if this stuff is a bit complicated...
 *
 * NOTE: Layers no longer determine the rendering order (which things are
 * drawn on top when to items overlap).  Now that is determined by depth
 * void SpaceLocation::set_depth ( double d)
 * and double SpaceLocation::get_depth()
 *
 * Standard depths include:
 * - DEPTH_STARS     (yes, you can draw things under the starfield if you want to)
 * - DEPTH_HOTSPOTS
 * - DEPTH_LINES
 * - DEPTH_SHOTS
 * - DEPTH_SHIPS
 * - DEPTH_EXPLOSIONS
 * - DEPTH_PRESENCE
 *
 * \section attributes_sec Attributes
 *
 * Currently, attributes are rather incomplete.  But eventually Query will
 * be modified to replace the layers parameter with an attributes parameter.
 * You can determine whether an object is derived from a common base-class
 * in the engine by checking it's attributes.  In addition, the following
 * attributes are defined at the moment:
 * ATTRIB_SYNCHED        describes the objects role in a network game
 * ATTRIB_INGAME         should generally be true
 * ATTRIB_TARGET         true if the item is a valid target
 * ATTRIB_FOCUS          true if the item is a camera focus (this attribute is not synched in network games)
 * ATTRIB_STANDARD_INDEX if true then SpaceObject::calculate will set sprite_index
 * ATTRIB_STRICT_RECT    if true then the item should only draw within its rectangle
 *
 * \section unit_used_sec Units used in TW
 *
 * For many things, TW uses 2 different kinds of units - external units and
 * internal units.  Internal units are used in physics calculations and
 * everything else in-game.  External units are used in .ini files for
 * configuration purposes.  Units are converted when they are read from .ini
 * files.  Here is a list of unit types, their internal representations,
 * external representations, and conversions.
 *
 * Time: internally milliseconds, externally SC2-time-units.
 * int ms = scale_frames(sc2_time);
 * Warning: the conversion is non-linear and rather weird.
 * Note: Some .ini files use milliseconds externally instead of SC2-time-units
 *
 * Angle: internally Radians, externally Degrees.
 * double radians = degrees * ANGLE_RATIO;
 *
 * Turning-rate: internally radians / millisecond, externally
 * SC2-turning-units.
 * double turning = scale_turning(sc2_turning);
 * Warning: the conversion is non-linear and rather weird.
 *
 * Distance: internally game-pixels, externally range-units.
 * double game_pixels = scale_range(range_units);
 *
 * Velocity: internally game-pixels per millisecond, externally
 * SC2-velocity-units.
 * double vel = scale_velocity ( ext_veloc );
 *
 * Acceleration: internally game-pixels per millisecond per millisecond,
 * externally... for most things it's in SC2-velocity-units per SC2-frames,
 * but for ships it's in a combination of an SC2-velocity-units # and an
 * SC2-time-units #.  It's kinda funky, but that's the format of the data
 * we ripped from SC2 data files.
 * double accel = scale_acceleration( sc2_vel_units_per_sc2_frame );
 * double ship_accel = scale_acceleration ( sc2_vel_units, sc2_time_units);
 * Warning: If SC2-time-units are used then the conversion is non-linear and
 * weird.
 *
 */

/*!
 *
 * \page howto_document Document your code
 *
 * \section motivation Motivation
 *
 * The document you are reading now was generated using
 * <a href="http://www.doxygen.org/">Doxygen</a>.
 * It follows in the tradition of
 * <a href="http://www.literateprogramming.com/">literal programming</a>,
 * the goal of which is to keep documentation in the source
 * code, when practical. This way, the documentation will
 * not be outdated or unmaintained.
 *
 * \section interfaces Commenting interfaces
 *
 * Concise comments are prefered, as long as the explanation
 * is correct, is not open to interpretation and does not
 * asume extensive knowledge of other parts of the system.
 *
 * By interface, we mean all content of a header file which
 * is available from a C++ source file, and could result in
 * compile errors if removed.
 * When you comment a header file, you need to take care
 * of a few, minor things in order to produce readable
 * documentation using Doxygen. The basic guidelines for
 * this project are:
 *
 * - Use three slashes when commenting part of an interface,
 *   rather than two, followed
 *   by a space and then the actual comment.
 *   The first sentence you write, terminated by a period,
 *   will be the brief description. After that, you can
 *   write a longer, more detailed description.
 *   The brief description will be shown in overviews,
 *   so it should be no more than a single line.
 *   It is possible to document virtually all parts of an interface,
 *   so it is not limited to classes.
 *
 * Example:
 * \code
 * / / / Takes care of displaying the map and game-data on the screen.
 * / / /
 * / / / The display is divided into two main sections: the game area,
 * / / / which displays the tiles of the game board, and units on them,
 * / / / and the side bar, which appears on the right hand side. The side bar
 * / / / display is divided into three sections.
 * class display {
 *    ...
 * };
 * \endcode
 *
 * - Do not refer to multiple objects of the type "Manager"
 *   as "Managers" or "manager". Instead, say "Manager objects".
 *   Doxygen will automatically link to class documentation whenever
 *   it finds class names in comments,
 *   but will not do so if you do not use their proper names.
 *
 * - Many <a href="http://www.stack.nl/~dimitri/doxygen/commands.html">Doxygen commands</a>
 *   can be used in comments to enhance the generated documentation and structure the comments.
 *   There is a balance between readable autogenerated documentation and
 *   readable code, so beware of overdoing it.
 *
 * Example:
 * \code
 * / / / \param a an integer dividend
 * / / / \param b an integer divisor, which must not be zero
 * / / / \returns a / b
 * / / /
 * / / / \pre b != 0
 * / / / \post divide' = a / b
 * / / /
 * / / / \throws std::runtime_error
 * / / / \todo this has not been peer reviewed yet
 * int divide(int a,int b) {
 * 	return a / b;
 * }
 * \endcode
 *
 */
