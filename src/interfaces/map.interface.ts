interface RotaMap {
    name: string;
    layers: Map<string, Layer>;
    bioms: Array<number>;
    map_weight: Map<Mode, number>;
    mapvote_weights: Map<Mode, number>;
    distances: Map<RotaMap, number>;
    neighbors: Array<RotaMap>;
    neighbor_count: number;
    lock_time: number;
    current_lock_time: number;
    lock_time_modifier: Map<Mode, number>;
    vote_weights: Map<Mode, number>;
    //for optimizer
    distribution: number;
    cluster_overlap: number;
    //for layer lock time
    mapvote_weight_sum: number;
    layer_locktime: number;
    locked_layers: Array<LockedLayer>;
    sigmoid_values: any; //was ist das ?

    /**
     * Locking a layer for the set layer_locktime, removing it from the available layers
     * @param {Layer} layer
     */
    lock_layer(layer: Layer): void;

    /**
     * calculating new mapvote weight sum for a specific mode
     * @param {string} mode
     */
    new_weight(mode: Mode): void;

    /**
     * resetting the layer locktime for every currently locked layer, making every layer available again.
     */
    reset_layer_locktime(): void;

    /**
     * decreasing the layer locktime by one for every locked layer.
     * re-adding the layer to the for this map available layers if no locktime is left.
     */
    decrease_layer_lock_time(): void;

    /**
     * adding a new layer as a property of this map
     * @param {Layer} layer
     */
    add_layer(layer: Layer): void;

    /**
     * decreasing the locktime by one
     */
    decrease_lock_time(): void;

    /**
     * setting the current locktime of this map to the saved standard lock time
     */
    update_lock_time(): void;

    /**
     * Calculating the mapvote weight for a specific mode.
     * Calculating for every for this map available mode, if no mode given.
     * @param {string} mode optional
     * @returns
     */
    add_mapvote_weights(mode: Mode): void;

    /**
     * calculate layervotes to weights
     * @param {string} mode mode for which the vote weight should be calculated
     */
    calculate_vote_weights_by_mode(mode: Mode): void;

    /**
     * calculates mapweight for given mode based on given params
     * @param {string} mode Mode for which the weight is to be calculated
     * @param {[float]} params Array of parameters for the calculation
     */
    calculate_map_weight(mode: Mode, params: Array<number>): number;
}
