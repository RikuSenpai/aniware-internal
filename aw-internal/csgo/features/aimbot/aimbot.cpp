#include "../../csgo.hpp"

namespace aimbot
{
	bool is_valid( player_t* pl )
	{
		if ( pl->IsDormant() )
			return false;

		if( pl == ctx::client.local )
			return false;

		if ( !pl->is_alive() || pl->get_flags().has_flag( FL_FROZEN ) )
			return false;

		if ( pl->is_immune() )
			return false;

		return true;
	}

	bool can_shoot( weapon_t* weapon )
	{
		if ( !weapon || weapon->get_ammo() <= 0 )
			return false;
		
		if ( weapon->get_next_primary_attack() > ctx::csgo.globals->curtime )
			return false;

		if ( weapon->get_definition_index() == WEAPON_REVOLVER && weapon->ready_time() > ctx::csgo.globals->curtime )
			return false;

		return true;
	}

	void select_points( AimbotData& data )
	{
		auto aim_body = config::get< bool >( ctx::cfg.aim_body ) ? 2 : 0;

		for ( auto i = aim_body; i < HITBOX_MAX; i++ )
		{
			auto point = data.pl->get_hitbox_pos( i );

			if ( !point.zero() )
			{
				data.points.push_back( point );
			}
		}
	}

	void filter_angles( AimbotData& data )
	{
		if ( data.points.empty() )
			return;

		for ( auto i = 0; i < data.points.size(); i++ )
		{
			if ( !data.points.at( i ).valid() )
			{
				data.points.erase( std::begin( data.points ) + i );
			}
		}
	}

	void select_angles( AimbotData& data )
	{
		ctx::client.cmd->viewangles = math::calc_angle( ctx::client.local->get_eye_pos(), data.points.front() );

		if ( !config::get< bool >( ctx::cfg.aim_silent ) )
		{
			ctx::csgo.engine->SetViewAngles( ctx::client.cmd->viewangles );
		}

		if ( config::get< bool >( ctx::cfg.aim_shoot) )
		{
			ctx::client.cmd->buttons.add_flag( IN_ATTACK );
		}
	}

	void work()
	{
		if ( !config::get< bool >( ctx::cfg.aim_enable ) )
			return;

		if ( !ctx::client.cmd )
			return;

		if ( !ctx::client.local || !ctx::client.local->is_alive() )
			return;

		auto weapon = entity_t::get< weapon_t >( ctx::client.local->get_weapon_handle() );

		if ( !can_shoot( weapon ) )
			return;

		game::for_every_player( []( player_t * pl ) -> bool {
			if ( !is_valid( pl ) )
				return false;

			AimbotData data( pl );
	
			select_points( data );
			filter_angles( data );
			select_angles( data );

			if ( !data.points.empty() )
				data.points.clear();

			return false;
		}, ( config::get< bool >( ctx::cfg.aim_friendly ) ? game::NO_FLAG : game::ENEMY_ONLY ) );
	}
}